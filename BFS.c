#pragma config(StandardModel, "EV3_REMBOT")

/*************************** STRUKTUR DATA ***************************/

typedef struct {
	int num_branch;					// jumlah anak
	int index_child[3];			// indeks node anak
	int active_branch;			// jumlah indeks aktif (bukan jalan buntu)
	int index_parent;				// indeks node orang tua
} node;

/*************************** KONSTANTA GLOBAL ***************************/

const int MAXNODE = 100;		// jumlah maksimal node
int STOPSECOND = 200;				// waktu berhenti untuk setiap langkah

/*************************** VARIABLE GLOBAL ***************************/

node tn[MAXNODE];							// array of node
int ptr_new_node = 0;					// indeks bagi node baru
int ptr_current_node = -1;		// indeks node yang sedang diperiksa
bool finished = false;				// true jika permainan selesai
int ptr_parent = -1;					// indeks node orang tua, digunakan saat mengekspansi node
int ind_child = -1;						// indeks anak ke-sekian, digunakan saat mengekspansi node

/*************************** PREDIKAT ***************************/

// Mengembalikan true jika color sensor menemukan warna biru
bool found_blue() {
	return getColorName(S3) == colorBlue;
}
// Mengembalikan true jika color sensor menemukan warna hijau
bool found_green() {
	return getColorName(S3) == colorGreen;
}
// Mengembalikan true jika color sensor menemukan warna merah
bool found_red() {
	return getColorName(S3) == colorRed;
}
// Mengembalikan true jika color sensor menemukan warna hitam
bool found_black() {
	return getColorName(S3) == colorBlack;
}
// Mengembalikan true jika color sensor menemukan warna putih
bool found_white() {
	return getColorName(S3) == colorWhite;
}
// Mengembalikan true jika node yang sedang diperiksa (ptr_curent_node) belum diekspansi
bool is_expanded() {
	bool isexp = false;
	for(int i = 0; i < tn[ptr_current_node].num_branch; i++)		 {
		if(tn[ptr_current_node].index_child[i] != -1) {
			isexp = true;
		}
	}
	return isexp;
}
// Mengembalikan true jika cabang aktif (tidak menemui jalan buntu)
bool is_active_branch(int i) {
	return ((tn[ptr_current_node].index_child[i] != -1) && (tn[tn[ptr_current_node].index_child[i]].active_branch > 0));
}

/*************************** GETTER ***************************/

// Mengambalikan urutan anak dari node yang sedang diperiksa
int get_child_no() {
	int p = tn[ptr_current_node].index_parent;
	int i = 0;
	while (tn[p].index_child[i] != ptr_current_node) i++;
	return i;
}
// Mengambalikan jumlah anak dari orang tua node yang sedang diperiksa
int get_num_child() {
	int p = tn[ptr_current_node].index_parent;
	return tn[p].num_branch;
}

/*************************** PROSEDUR ***************************/

// Prosedur untuk memutar robot sejauh 'degree' derajat,
// dengan menjalankan motor kiri sebesar 'left' dan motor kanan sebesar 'right'
void move_gyro_left_right_degree(int left, int right, int degree) {
	resetGyro(S2);
	if(degree > 0) {
		repeatUntil(getGyroDegrees(S2) > degree) {
			setMotorSpeed(motorC, right);		// right
			setMotorSpeed(motorB, left);		// left
		}
	}
	else {
		repeatUntil(getGyroDegrees(S2) < degree) {
			setMotorSpeed(motorC, right);		// right
			setMotorSpeed(motorB, left);		// left
		}
	}
}
// Memutar posisi robot untuk persiapan melakukan putaran (untuk menghitung cabang atau
// bergerak ke cabang lain)
void get_to_rotation_position() {
	move_gyro_left_right_degree(50, 0, 90);
}
// Memutar balik posisi robot untuk persiapan melakukan backtrack
void get_to_backtrack_position() {
	resetGyro(S2);
	repeatUntil(getGyroDegrees(S2) < -270) {
		setMotorSpeed(motorC, 50);		// right
		setMotorSpeed(motorB, 0);		// left
	}
}
// Inisialisasi tn
void init_table_node() {
	for(int i = 0; i < MAXNODE; i++) {
		tn[i].num_branch = -1;
		for(int j = 0; j < 3; j++) {
			tn[i].index_child[j] = -1;
		}
		tn[i].active_branch = -1;
		tn[i].index_parent = -1;
	}
}
// Menampilkan status node ke layar
void display_status() {
	displayString(5, "   CURRENT NODE: %d", ptr_current_node);
	if(ptr_parent != -1)
		displayString(6, "   CHILD NO %d OF NODE %d", ind_child + 1, ptr_parent);
}
// Menambahkan node pertama ke dalam tn
void insert_start_node() {
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   FOUND THE START NODE");
	display_status();
	int index_start = ptr_new_node++;
	tn[index_start].num_branch = 1;
	tn[index_start].index_child[0] = ptr_new_node++;
	tn[index_start].active_branch = 1;
	ptr_current_node = 1;
	tn[1].index_parent = index_start;
}
// Prosedur untuk menjalankan motor kiri sebesar 'left' dan motor kanan sebesar 'right'
void move_motor_left_right(int left, int right) {
	motor[leftMotor]  = left;
	motor[rightMotor] = right;
}
// Mencari jalan pada saat pertama kali menemukan start node
void search_for_track() {
	while(getColorName(S3) != colorBlack){
		move_motor_left_right(30,30);
	}
	sleep(500);
}
// Prosedur untuk membuat robot berhenti bergerak selama 's' milidetik
void stop_for_second(int s){
	motor[leftMotor]  = 0;
  motor[rightMotor] = 0;
	sleep(s);
}
// Menelusuri jalan hitam sampai color sensor menemukan node apapun
// dan menampilkan node apa yang ditemukan
void move_until_found_node() {
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   SEARCHING NODE");
	display_status();
	int stop = false;
	while (!stop) {
		if(found_white()) {
	  	// counter-steer right:
	    move_motor_left_right(15,55);
	  }
	  else if (found_black()) {
	    // counter-steer left:
	   	move_motor_left_right(55,15);
		}
		else if (found_blue()) {
			eraseDisplay();
			displayString(1, "STATUS:");
			displayString(2, "   FOUND BLUE");
	    stop = true;
		}
		else if (found_green()) {
			eraseDisplay();
			displayString(1, "STATUS:");
			displayString(2, "   FOUND GREEN");
	    stop = true;
		}
		else if (found_red()) {
			eraseDisplay();
			displayString(1, "STATUS:");
			displayString(2, "   FOUND RED");
	    stop = true;
		}
		else {
			move_motor_left_right(30,30);
		}
	}
	display_status();
}
// Fungsi yang mengembalikan jumlah cabang pada suatu titik hijau
int count_branch() {
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   COUNTING BRANCH");
	display_status();
	int count = 0;
	bool black = false;
	resetGyro(S2);
	repeatUntil(getGyroDegrees(S2) < -360) {
		setMotorSpeed(motorC, 50);		// right
		setMotorSpeed(motorB, 0);		// left
		if(getColorName(S3) == colorBlack) {
			black = true;
		}
		if((getColorName(S3) == colorWhite) && (black)) {
			count++;
			black = false;
		}
	}
	stop_for_second(STOPSECOND);

	if((count <= 4) && (count >= 1)) {
		displayString(3, "      %d BRACHES", count - 1);
	}
	else {
		displayString(3, "      ERROR: count_branch");
		stop_for_second(STOPSECOND*100);
	}
	return count - 1;
}
// Menemukan cabang terkanan pada suatu persimpangan
void find_rightmost_branch() {
	while(getColorName(S3) != colorBlack){
  	move_motor_left_right(15,55);
	}
}
// Menambahkan node pada saat sedang ekspansi node
void insert_node_child() {
	tn[ptr_current_node].index_parent = ptr_parent;
	if(found_green()) {
		get_to_rotation_position();
		stop_for_second(STOPSECOND);
		int b = count_branch();
		stop_for_second(STOPSECOND);
		tn[ptr_current_node].num_branch = b;
		tn[ptr_current_node].active_branch = b;
		tn[ptr_parent].index_child[ind_child] = ptr_current_node;
		stop_for_second(STOPSECOND);
		get_to_backtrack_position();
		stop_for_second(STOPSECOND);
		find_rightmost_branch();
		move_until_found_node();
		get_to_rotation_position();
		stop_for_second(STOPSECOND);
	}
	else if(found_red()) {
		tn[ptr_current_node].num_branch = 0;
		tn[ptr_parent].active_branch--;
		stop_for_second(STOPSECOND);
		get_to_rotation_position();
		stop_for_second(STOPSECOND);
		get_to_backtrack_position();
		stop_for_second(STOPSECOND);
		find_rightmost_branch();
		move_until_found_node();
		get_to_rotation_position();
		stop_for_second(STOPSECOND);
	}
	else if(found_blue()) {
		finished = true;
		tn[ptr_parent].index_child[ind_child] = ptr_current_node;
	}
	display_status();
}
// Menyebrangi cabang paling kanan dari suatu persimpangan
void skip_branch() {
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   SKIPPING INACTIVE BRANCH");
	display_status();
	while(getColorName(S3) != colorBlack){
		move_motor_left_right(15,55);
	}
	while(getColorName(S3) != colorWhite){
		move_motor_left_right(15,55);
	}
}
// Mengekspansi node yang sedang diperiksa
void expand_node() {
	// Basis
	if(!is_expanded()) {
		eraseDisplay();
		displayString(1, "STATUS:");
		displayString(2, "   EXPANDING NODE");
		display_status();
		stop_for_second(STOPSECOND);
		ptr_parent = ptr_current_node;
		for(int i = 0; i < tn[ptr_parent].num_branch; i++) {
			if(!finished) {
				ind_child = i;
				ptr_current_node = ptr_new_node++;
				find_rightmost_branch();
				move_until_found_node();
				insert_node_child();
			}
		}
		if(!finished) {
			if(tn[ptr_parent].active_branch == 0)
				tn[tn[ptr_parent].index_parent].active_branch--;
			ind_child = -1;
			ptr_current_node = tn[ptr_parent].index_parent;
			ptr_parent = -1;
			find_rightmost_branch();
			stop_for_second(STOPSECOND);
			move_until_found_node();
			get_to_rotation_position();
			stop_for_second(STOPSECOND);
		}
	}
	// Rekurens
	else {
		eraseDisplay();
		displayString(1, "STATUS:");
		displayString(2, "   NODE IS ALREADY EXPANDED");
		display_status();
		stop_for_second(STOPSECOND);
		int i = 0;
		while ((i < tn[ptr_current_node].num_branch) && (!finished)) {
			if(ptr_current_node == 0)
				get_to_backtrack_position();
			find_rightmost_branch();
			if(is_active_branch(i)) {
				eraseDisplay();
				displayString(1, "BRANCH %d IS ACTIVE", i);
				stop_for_second(STOPSECOND);
				ptr_current_node = tn[ptr_current_node].index_child[i];
				move_until_found_node();
				get_to_rotation_position();
				expand_node();
			}
			else {
				eraseDisplay();
				displayString(1, "BRANCH %d IS SKIPPED", i);
				stop_for_second(STOPSECOND);
				skip_branch();
			}
			i++;
		}
		if(ptr_current_node != 0 && !(finished)) {
			ptr_current_node = tn[ptr_current_node].index_parent;
			find_rightmost_branch();
			stop_for_second(STOPSECOND);
			move_until_found_node();
			get_to_rotation_position();
			stop_for_second(STOPSECOND);
		}
	}
}

/*************************** PROGRAM UTAMA ***************************/
task main()
{
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   GAME START");
	display_status();
	init_table_node();
// Mencari node start
	while(!found_blue()) {
		move_motor_left_right(30,30);
	}
	insert_start_node();
	search_for_track();
	move_until_found_node();
// Ekspansi node pertama (node 1)
	get_to_rotation_position();
	stop_for_second(STOPSECOND);
	int b = count_branch();
	tn[ptr_current_node].num_branch = b;
	tn[ptr_current_node].active_branch = b;
// Mengekspansi node akar (node 0) sampai menemukan node finish
	while(!finished) {
		expand_node();
	}
// Sudah sampai di node finish
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   FINISHED");
	displayString(3, "   GO HOME IN 5 SECONDS");
// Menampilkan jalur yang dilalui
	displayString(6, "PATHS:");
	int k = ptr_current_node;
	int l = 0;
	while(k != -1) {
			displayString(7+l, "%d", k);
			k = tn[k].index_parent;
			l++;
	}
	stop_for_second(5000);

// Kembali ke node start
	ptr_parent = -1;
	int num_child = 0;
	int child_no = 0;
	get_to_rotation_position();
	get_to_backtrack_position();
	find_rightmost_branch();
	while(!found_blue()) {
		move_until_found_node();
		if(!found_blue()) {
			get_to_rotation_position();
			find_rightmost_branch();
			num_child = get_num_child() - 1;
			child_no = get_child_no();
			while (num_child != child_no) {
				skip_branch();
				find_rightmost_branch();
				num_child--;
			}
			ptr_current_node = tn[ptr_current_node].index_parent;
		}
	}
// Sudah kembali ke node awal
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   FINISHED GO HOME");
	displayString(3, "   GAME OVER");
}
