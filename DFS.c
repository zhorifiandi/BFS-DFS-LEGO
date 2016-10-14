#pragma config(StandardModel, "EV3_REMBOT")

// Prosedur untuk membuat robot berhenti bergerak selama 's' milidetik
void stop_sec(int s){
	motor[leftMotor]  = 0;
  motor[rightMotor] = 0;
	sleep(s);
}

// Prosedur untuk menjalankan motor kiri sebesar 'left' dan motor kanan sebesar 'right'
void move_motor_left_right(int left, int right) {
	motor[leftMotor]  = left;
	motor[rightMotor] = right;
}

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

// Fungsi yang mengembalikan jumlah cabang pada suatu titik hijau
int count_branch() {
	eraseDisplay();
	displayString(1, "STATUS:");
	displayString(2, "   COUNTING BRANCH");
	int left = 0;
	int right = 50;
	int degree = -360;
	int count = 0;
	bool black = false;
	resetGyro(S2);
	repeatUntil(getGyroDegrees(S2) < degree) {
		setMotorSpeed(motorC, right);		// right
		setMotorSpeed(motorB, left);		// left
		if(getColorName(S3) == colorBlack) {
			black = true;
		}
		if((getColorName(S3) == colorWhite) && (black)) {
			count++;
			black = false;
		}
	}

	if((count <= 4) && (count >= 2))
		displayString(3, "      %d BRACHES", (count - 1));
	else
		displayString(3, "ERROR: count_branch");

	return count - 1;
}


// MAIN PROGRAM
task main()
{
	int stop_second = 100;	// waktu berhenti sebagai parameter prosedur stop_sec
	bool finished = false;
	int table[10];
	int index = 0;
	int branch;
	bool is_backtrack = false;

	for(int i = 0; i < 10; i++) {
		table[i] = 0;
	}

//============================ Searching for blue square (start point)
	while(getColorName(S3) != colorBlue){
		move_motor_left_right(30,30);
	}
	eraseDisplay();
	displayString(1, "STATUS: ");
	displayString(2, "   FOUND THE START");
	while(getColorName(S3) != colorBlack){
		move_motor_left_right(30,30);
		sleep(1000);
	}
	eraseDisplay();

//============================ Running on line while not finished
  while(!finished)
  {
  	displayString(1, "STATUS: ");
  	displayString(2, "   RUNNING");
//-------------- WHITE
    if(getColorName(S3) == colorWhite)
    {
      // counter-steer right:
    	move_motor_left_right(15,55);
    }
//-------------- BLACK
    else if(getColorName(S3) == colorBlack)
    {
      // counter-steer left:
    	move_motor_left_right(55,15);
    }
//-------------- GREEN
    else if(getColorName(S3) == colorGreen){
    	eraseDisplay();
    	displayString(1, "STATUS:");
    	displayString(2, "   FOUND GREEN");
    	move_gyro_left_right_degree(50, 0, 90);
    	stop_sec(stop_second);


   		if(!is_backtrack || index == 0) {
   			branch = count_branch();
   			index++;
   			table[index] = branch;
   		}
   		if(is_backtrack) {
   			is_backtrack = false;
   		}
   		if(table[index] == 0) {
   			index--;
   			table[index]--;
   			is_backtrack = true;
   		}

			stop_sec(stop_second);

    	// counter-steer right:
    	while(getColorName(S3) != colorBlack){
    		move_motor_left_right(15,55);
    	}
    	eraseDisplay();
  	}
//-------------- BLUE
  	else if(getColorName(S3) == colorBlue){
    	move_motor_left_right(0,0);
      finished = true;
  	}
//-------------- RED
  	else if(getColorName(S3) == colorRed){
  		eraseDisplay();
    	displayString(1, "STATUS:");
    	displayString(2, "   FOUND RED");
    	stop_sec(stop_second);
    	move_gyro_left_right_degree(-50, 50, -180);
    	stop_sec(stop_second);
    	eraseDisplay();
    	table[index]--;
    	is_backtrack = true;
  	}
//-------------- OTHER
 	 	else {
 			move_motor_left_right(30,30);
		}
  }

//============================ Finished running
  if(finished){
  	eraseDisplay();
  	displayString(1, "STATUS:");
  	displayString(2, "   FINISHED");
  	displayString(3, "   GO HOME IN 10 SECONDS");
  	displayString(4, "PATH:");
  	for(int i = 1; i <= index; i++) {
  		displayString(4+i, "  %d ", table[i]);
  	}
	}
	stop_sec(10000);

//============================ Running on line to go home
	move_gyro_left_right_degree(-50, 50, -180);
	finished = false;
	eraseDisplay();
  while(!finished)
  {
  	displayString(1, "STATUS: ");
  	displayString(2, "   GO HOME");
//-------------- WHITE
    if(getColorName(S3) == colorWhite)
    {
      // counter-steer right:
    	move_motor_left_right(15,55);
    }
//-------------- BLACK
    else if(getColorName(S3) == colorBlack)
    {
      // counter-steer left:
    	move_motor_left_right(55,15);
    }
//-------------- GREEN
    else if(getColorName(S3) == colorGreen){
    	eraseDisplay();
    	displayString(1, "STATUS:");
    	displayString(2, "   FOUND GREEN");
    	stop_sec(stop_second);

    	move_gyro_left_right_degree(50, 0, 90);
    	stop_sec(stop_second);

    	// counter-steer right:
    	for(int i = 0; i < table[index]; i++) {
		    while(getColorName(S3) != colorBlack){
	    		move_motor_left_right(15,55);
	    	}
	    	while(getColorName(S3) != colorWhite){
	    		move_motor_left_right(15,55);
	    	}
    	}
			while(getColorName(S3) != colorBlack){
	    		move_motor_left_right(55,15);
	    	}
	    index--;
    	eraseDisplay();
  	}
//-------------- BLUE
  	else if(getColorName(S3) == colorBlue){
    	move_motor_left_right(0,0);
      finished = true;
  	}
//-------------- OTHER
 	 	else {
 			move_motor_left_right(30,30);
		}
  }

//============================ Finished go home
	if(finished){
		eraseDisplay();
		displayString(1, "STATUS:");
		displayString(2, "   FINISHED GO HOME");
		displayString(3, "   PROGRAM OVER");
	}
}
