#include "main.h"

/**
 * A callback function for LLEMU's center button.
 *
 * When this callback is fired, it will toggle line 2 of the LCD text between
 * "I was pressed!" and nothing.
 */

//motor direction
#define LEFT_BASE_REVERSED false
#define RIGHT_BASE_REVERSED true
#define LEFT_BASE_2_REVERSED false
#define RIGHT_BASE_2_REVERSED true

#define LIFT_REVERSED false

#define LEFT_ROLLER_REVERSED false
#define RIGHT_ROLLER_REVERSED true

#define TRAY_REVERSED false

//motor ports
#define LEFT_BASE_PORT 3 //was 8, now 3
#define RIGHT_BASE_PORT 8 //was 3, now 8
#define LEFT_BASE_2_PORT 6 //was 4, then 11, now 6
#define RIGHT_BASE_2_PORT 11 //was 6, now 11

#define LIFT_PORT 12

#define LEFT_ROLLER_PORT 14 //was 18, then 13 (port dead), now 14
#define RIGHT_ROLLER_PORT 17 //was 13, then 18, now 17

#define TRAY_PORT 1 //was 19, now 1


//sets up controller and motor objects
pros::Controller master(pros::E_CONTROLLER_MASTER);
pros::Motor left_mtr(LEFT_BASE_PORT, pros::E_MOTOR_GEARSET_18, LEFT_BASE_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor right_mtr(RIGHT_BASE_PORT, pros::E_MOTOR_GEARSET_18, RIGHT_BASE_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor left_mtr2(LEFT_BASE_2_PORT, pros::E_MOTOR_GEARSET_18, LEFT_BASE_2_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor right_mtr2(RIGHT_BASE_2_PORT, pros::E_MOTOR_GEARSET_18, RIGHT_BASE_2_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor lift_mtr(LIFT_PORT, pros::E_MOTOR_GEARSET_18, LIFT_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor left_roller_mtr(LEFT_ROLLER_PORT, pros::E_MOTOR_GEARSET_18, LEFT_ROLLER_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);
pros::Motor right_roller_mtr(RIGHT_ROLLER_PORT, pros::E_MOTOR_GEARSET_18, RIGHT_ROLLER_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);

pros::Motor tray_mtr(TRAY_PORT, pros::E_MOTOR_GEARSET_18, TRAY_REVERSED, pros::E_MOTOR_ENCODER_DEGREES);




int trueSpeed[] =   {0,  3,  3,  3,  3,  3, 10, 10, 10, 10,
                    10, 10, 10, 10, 12, 12, 12, 12, 12, 12,
                    14, 14, 14, 14, 14, 14, 20, 20, 20, 20,
                    20, 20, 20, 25, 25, 25, 25, 25, 25, 25,
                    30, 30, 30, 30, 30, 30, 30, 30, 30, 30,
                    45, 45, 45, 45, 45, 45, 45, 45, 45, 45,
                    55, 55, 55, 55, 55, 55, 55, 55, 55, 55,
                    66, 66, 66, 66, 66, 66, 66, 66, 66, 66,
                    79, 79, 79, 79, 79, 79, 79, 79, 79, 79,
                    89, 89, 89, 89, 89, 89, 89, 89, 89, 89,
                    95, 95, 95, 99, 99, 99, 99, 104,104,104,
                    107,107,107,113,113,113,113,113,116,116,
                    116,116,123,123,123,127,127,127};

/*
int trueSpeed[] =  {0,  0,  3,  3,  3,  3,  10, 10, 10, 10,
									  10, 10, 10, 10, 12, 12, 12, 12, 12, 12,
									  14, 14, 14, 14, 14, 14, 20, 20, 20, 20,
									  20, 20, 20, 25, 25, 25, 25, 25, 25, 30,
									  30, 30, 30, 30, 30, 45, 45, 45, 45, 45,
									  45, 45, 45, 55, 55, 55, 55, 66, 66, 66,
									  66, 79, 79, 79, 79, 79, 89, 89, 89, 89,
									  89, 89, 89, 95, 95, 95, 99, 99, 99, 99,
									  104,104,104,107,107,107,113,113,113,113,
									  113,116,116,116,116,123,123,123,127,127,
									  127};
*/

int returnTrueSpeed (int controllerVal) {
	int trueSpeedVal = 0;
	if (controllerVal >= 0) {
    trueSpeedVal = (int)(100.0/127*(trueSpeed[abs(controllerVal)]));
	}
	else if (controllerVal < 0) {
    trueSpeedVal = (int)(100.0/127*(-trueSpeed[abs(controllerVal)]));
	}
	return trueSpeedVal;
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//debugging variables
bool const printBaseVals = false;
bool const printLiftVals = false;
bool const printRollerVals = false;
bool const printTrayVals = false;
bool const printAngle = false;
bool const printTime = false;
bool const printPosTrackVal = false;
bool const printPIDBasedAutonVal1 = false;

///////////////////////////////////////////////////////////////////////////////////////////////////////

//variables for lift PID
static const double liftKp = 1.5;
static double liftSetVal = 0;
static double liftErr = 0;
static double currLiftVal = 0;
static bool liftHold = false;
static double autonLiftSetVal = -1100;
static int autonLift = 0;

void updateLiftPID (void* param) {
  while (true) {
    //lift
		currLiftVal = lift_mtr.get_position();
		liftErr = liftSetVal - currLiftVal;
		//lift_mtr = liftErr * liftKp;
		if (printLiftVals) {
			pros::lcd::print(3, "liftHold: %d", liftHold);
			pros::lcd::print(4, "currLiftVal: %d", currLiftVal);
			pros::lcd::print(5, "liftSetVal: %d", liftSetVal);
			pros::lcd::print(6, "liftErr: %d", liftErr);
			pros::lcd::print(7, "lift_mtr: %d", (liftErr * liftKp));
		}
		//Manual lift code
		if (master.get_digital(DIGITAL_R1)) {
			liftHold = false;
			lift_mtr = -127;
		}
	  else if (master.get_digital(DIGITAL_R2)) {
			liftHold = false;
			lift_mtr = 127;
		}
    else if (autonLift == 1) {
      liftHold = false;
      liftSetVal = autonLiftSetVal;
    }
    else if (autonLift == 2) {
      liftHold = false;
      liftSetVal = 0;
    }
		else if (liftHold == false) {
			liftHold = true;
			liftSetVal = currLiftVal;
		}
		//lift PID hold
		if (liftHold) {
			lift_mtr = liftErr * liftKp;
		}

    pros::Task::delay(10);
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//variables for left roller PID
static const double leftRollerKp = 1.5;
static double leftRollerSetVal = 0;
static double leftRollerErr = 0;
static double currLeftRollerVal = 0;
//variables for right roller PID
static const double rightRollerKp = 1.5;
static double rightRollerSetVal = 0;
static double rightRollerErr = 0;
static double currRightRollerVal = 0;
//shared roller variables
static bool rollerHold = false;
static int autonRoller = 0; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold

void updateRollerPID (void* param) {
  while (true) {
    //roller
		currLeftRollerVal = left_roller_mtr.get_position();
		leftRollerErr = leftRollerSetVal - currLeftRollerVal;
		currRightRollerVal = right_roller_mtr.get_position();
		rightRollerErr = rightRollerSetVal - currRightRollerVal;

		if (printRollerVals) {
			pros::lcd::print(3, "rollerHold: %d", rollerHold);
			pros::lcd::print(4, "currLeftRollerVal: %d", currLeftRollerVal);
			pros::lcd::print(5, "leftRollerSetVal: %d", leftRollerSetVal);
			pros::lcd::print(6, "leftRollerErr: %d", leftRollerErr);
			pros::lcd::print(7, "left_roller_mtr: %d", (leftRollerErr * leftRollerKp));
		}
		//Manual roller code
		if (master.get_digital(DIGITAL_L1) || (autonRoller == 1)) {
			rollerHold = false;
			left_roller_mtr = -127;
			right_roller_mtr = -127;
		}
	  else if (master.get_digital(DIGITAL_L2) || (autonRoller == 2)) {
			rollerHold = false;
			left_roller_mtr = 127;
			right_roller_mtr = 127;
		}
		else if ((rollerHold == false) || (autonRoller == 3)) {
			rollerHold = true;
			leftRollerSetVal = currLeftRollerVal;
			rightRollerSetVal = currRightRollerVal;
		}
    if (autonRoller == 4) {
			rollerHold = false;
			left_roller_mtr = -65;
			right_roller_mtr = -65;
		}
    if (autonRoller == 5) {
			rollerHold = false;
			left_roller_mtr = -25;
			right_roller_mtr = -25;
		}
		//roller PID hold
		if (rollerHold) {
			left_roller_mtr = leftRollerErr * leftRollerKp;
			right_roller_mtr = rightRollerErr * rightRollerKp;
		}

    pros::Task::delay(10);

  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//variables for tray PID
static double trayKp = 1.5;
static const double trayNormalKp = 1.5;
static double traySetVal = 0;
static double trayErr = 0;
static double currTrayVal = 0;
static bool trayHold = false;

//variables for the automated scoring sequence
static const double autoTrayKp = 0.37;
static const double autoTraySetVal = -1600;
static const double autonTraySetVal = -1000;
static double autonTrayKp = 0.10;
static bool autoTray = false;
static int autonTray = 0; //this is used during auton, if it's true, then the tray will go up from fast to slow using pid (just like the up button on the controller)

void updateTrayPID (void* param) {
  while (true) {
    //tray
		currTrayVal = tray_mtr.get_position();
		trayErr = traySetVal - currTrayVal;
    int diplayVal = (int) currTrayVal;
		if (printTrayVals) {
      pros::lcd::print(1, "trayKp: %f", trayKp);
      pros::lcd::print(2, "autoTray: %d, autonTray: %d", autoTray, autonTray);
			pros::lcd::print(3, "trayHold: %d", trayHold);
			pros::lcd::print(4, "currTrayVal: %d", diplayVal);
			pros::lcd::print(5, "traySetVal: %d", traySetVal);
			pros::lcd::print(6, "trayErr: %d", trayErr);
			pros::lcd::print(7, "tray_mtr: %d", (trayErr * trayKp));
		}
		//Manual tray code
		if (master.get_digital(DIGITAL_A)) {
			trayHold = false;
			tray_mtr = -127;
		}
	  else if (master.get_digital(DIGITAL_B)) {
			trayHold = false;
			tray_mtr = 127;
		}
		else if (master.get_digital(DIGITAL_X)) {
			trayHold = false;
			tray_mtr = -32;
		}
		else if (master.get_digital(DIGITAL_Y)) {
			trayHold = false;
			tray_mtr = 32;
		}
    else if (master.get_digital(DIGITAL_UP) || (autonTray == 1)) {
			trayHold = false;
			//traySetVal = currTrayVal;
      traySetVal = autoTraySetVal;

      if (currTrayVal < -800) {
        trayKp = autonTrayKp;
      } else {
        trayKp = autoTrayKp;
      }
      //tray PID hold
			tray_mtr = trayErr * trayKp;

		}
    else if (autonTray == 2) {
      trayHold = false;
      traySetVal = autonTraySetVal;
    }
    else if (autonTray == 3) {
      trayHold = false;
      traySetVal = 0;
    }
    else if (trayHold == false) {
			trayHold = true;
			traySetVal = currTrayVal;
      trayKp = trayNormalKp;
		}
		//tray PID hold
		if (trayHold) {
			tray_mtr = trayErr * trayKp;
		}

    pros::Task::delay(10);

  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////

//pos track variables

double currX = 0.0; //current robot x position
double currY = 0.0; //current robot y position
double currA = 0.0; //current robot heading

double dX = 0.0; //change in x
double dY = 0.0; //change in y
double dA = 0.0; //change in robot heading
double dR = 0.0; //hypotenuse of the triangle with legs being dX adn dY

int currLEnc = 0; //current left base encoder value
int currREnc = 0; //current right base encoder value
int lastLEnc = 0; //current left base encoder value
int lastREnc = 0; //current right base encoder value
int dLEnc = 0;
int dREnc = 0;

double rLCOR = 0; //(radius of Left Center Of Rotation) The distance from the IDEAL left encoder wheel position to the left center of rotation (when making a left turn)
double rRCOR = 0; //(radius of Right Center Of Rotation) The distance from the IDEAL right encoder wheel position to the right center of rotation (when making a right turn)
double rMCOR = 0; //(radius of Middle Center of Rotation) The distance from the IDEAL right encoder wheel position to the center of the rotation (when making a point turn)

const int refreshRate = 500; //in Hz

//encoder offset calculation variables
const double encLRDist = 11.6 * 360 / (4 * M_PI); //distance between the left and right base encoders
const double encCMidDist = 5.2 * 360 / (4 * M_PI); //distance from the center base encoder to its IDEAL position

//This function updates the robot's current X, Y position, and Angle
void updateRobotPos (void* param) {
  while (true) {
    //updating position
    currA += dA;
    dX = dR *cos(currA); //argument in radians
    dY = dR *sin(currA); //argument in radians
    currX += dX;
    currY += dY;
    if (printPosTrackVal) {
      pros::lcd::print(3, "X: %f, Y: %f", currX, currY);
      pros::lcd::print(4, "A(degrees): %f", (currA * (180/M_PI)));
      pros::lcd::print(5, "LEnc: %d, REnc: %d", currLEnc, currREnc);
    }

    currLEnc = (int) left_mtr2.get_position();
    currREnc = (int) right_mtr2.get_position();

    dR = (dLEnc + dREnc) / 2;
    dA = (dREnc - dLEnc) / encLRDist;

    dLEnc = currLEnc - lastLEnc;
    dREnc = currREnc - lastREnc;

    lastLEnc = currLEnc;
    lastREnc = currREnc;

    pros::Task::delay(1000/refreshRate);
  }
}

//this function resets the robot's current X, Y position, and Angle
void calibrate (double x, double y, double a = currA) {
  currX = x;
  currY = y;
  currA = a;
}

//convert any angle in radians to 0 <= targetAngle < 2*M_PI
/*
double makeAngleSimple (double a) {
  while (a < 0.0 || a >= 2*M_PI){
    if (a < 0){
      a += 2*M_PI;
    }
    if (a >= 2*M_PI){
      a -= 2*M_PI;
    }
  }
  return a;
}
*/
double makeAngleSimple (double a) {
  double remainder; //1.0 = 2*M_PI radians
  double quotient;
  int quotInt;

  quotient = a / (2*M_PI);
  quotInt = a / (2*M_PI);
  remainder = quotient - quotInt;
  if (remainder < 0) {
    remainder += 1;
  }

  return remainder*2*M_PI;
}

double targetA;
double turnToErr;
double turnToLastErr;
double turnToDeriv;

//returns positive motor power turning counterclockwise, negative motor power turning clockwise
double shortestTurn (double targetAngle, double turnToKp = 100.0, double turnToKi = 0.0, double turnToKd = 0.0) {
  bool clockwise = false; //false turns counterclockwise, true turns clockwise to reach targetAngle
  double currABounded; //basically another currA used so currA itself will not be changed
  double shortestTurnMotorPower;


  //turn these angles to between 0 and 2*M_PI radians
  targetAngle = makeAngleSimple(targetAngle);
  currABounded = makeAngleSimple(currA);

  //determine the direction of correction
  //separates negative and positive errors to prevent conflict
  if ((targetAngle - currABounded) >= 0){
    //currA always reaches targetAngle in less than or equal to M_PI degrees
    if ((targetAngle - currABounded) > M_PI) {
      clockwise = true;
    }
    else {
      clockwise = true;
    }
  }
  else if ((targetAngle - currABounded) < 0){
    //currA always reaches targetAngle in less than or equal to M_PI degrees
    if ((targetAngle - currABounded) > -M_PI) {
        clockwise = true;
    }
    else {
        clockwise = true;
    }
  }

  //angular correction in clockwise direction
  if (clockwise) {
    //make sure currA - targetAngle is less than M_PI and positive
    while ((currA < targetAngle) || (currA - targetAngle) > 2*M_PI){
      if (currA < targetAngle){
        currA += 2*M_PI;
      }
      if ((currA - targetAngle) > 2*M_PI){
        currA -= 2*M_PI;
      }
    }

    //pid working
    //caculates error and derivative
    //since direction is already determined at this point, maleAngleSimple() can be applied here
    turnToErr = currA - targetAngle; //positive clockwise error
    turnToDeriv = turnToErr - turnToLastErr;
    //prints stuff
    if (printAngle) {
      pros::lcd::print(6, "cA: %f, clw?: %d, tErr: %f", currA, clockwise, turnToErr);
    }
    //returns motor power
    shortestTurnMotorPower = -((turnToErr * turnToKp) + (turnToDeriv * turnToKd));
    return shortestTurnMotorPower;
  }
  //angular correction in counterclockwise direction
  else if (clockwise == false) {
    //make sure targetAngle - currA is less than or equal to M_PI and positive
    while ((targetAngle < currA) || (targetAngle - currA) > 2*M_PI){
      if (targetAngle < currA){
        currA -= 2*M_PI;
      }
      if ((targetAngle - currA) > 2*M_PI){
        currA += 2*M_PI;
      }
    }

    //pid working
    //caculates error and derivative
    //since direction is already determined at this point, maleAngleSimple() can be applied here
    turnToErr = targetAngle - currA;
    turnToDeriv = turnToErr - turnToLastErr;
    //prints stuff
    if (printAngle) {
      pros::lcd::print(6, "cA: %f, clw?: %d, tErr: %f", currA, clockwise, turnToErr);
    }
    //returns motor power
    shortestTurnMotorPower = (turnToErr * turnToKp) + (turnToDeriv * turnToKd);
    return shortestTurnMotorPower;
  }
}

//this function turns the robot to the desired angle for a specified time, and PID value
void turnTo (double tA, int t, double turnToKp = 5.0, double turnToKi = 0.0, double turnToKd = 2.0) {
  double turnToMotorPower;
  int time = 0;
  //turnToErr = currA - tA; //just for initialization, so while loop actually runs
  while (time < t) {
    turnToMotorPower = shortestTurn(tA);

    // left_mtr = -turnToMotorPower;
		// left_mtr2 = -turnToMotorPower;
	  // right_mtr = turnToMotorPower;
		// right_mtr2 = turnToMotorPower;

    if (printTime) {
      pros::lcd::print(7, "t: %d, mPower: %f", time, turnToMotorPower);
    }

    pros::Task::delay(10);
    time += 10;
  }
}

///////////////////////////////////////////////////////////////////////////////////////////////////////



//IMPORTANT: BEFORE USING PID-BASED AUTON, MAKE SURE TO TURN OFF THE TASK THAT UPDATES THE ROBOT POSITION FOR POS TRACK.
//YOU CAN FIND THIS IN initialize() BELOW

//PID-based auton stuff
//go a set amount of distance until the specified time is reached, or until errCutTime is reached
//errCutTime meaning: if the robot is very close to the specified distance, then operation will cut off before the specified time is reached

//maxAutonSpeed (80, someVal) limits the motor power to less than 80 in both directions
double maxAutonSpeed (int max, double val) {
  if (abs(val) > max) {
    if (val >= 0) {
      return abs(max)*1.0;
    } else {
      return abs(max)*-1.0;
    }
  } else {
    return val;
  }
}


double goDistKp = 0.05;
double goDistbaseLeft1Error;
double goDistbaseLeft2Error;
double goDistbaseRight1Error;
double goDistbaseRight2Error;

double goDistbaseLeft1Current;
double goDistbaseLeft2Current;
double goDistbaseRight1Current;
double goDistbaseRight2Current;

double goDistbaseLeft1SetDist;
double goDistbaseLeft2SetDist;
double goDistbaseRight1SetDist;
double goDistbaseRight2SetDist;

void goDistance (double setDist, double setKp, int time, int errCutTime, int maxSpeed = 80) {
  double goDistKp = setKp;
  int t = 0;
  int e = 0;

  //sets the absolute zero on the motor encoder to current value
  left_mtr.tare_position();
  left_mtr2.tare_position();
  right_mtr.tare_position();
  right_mtr2.tare_position();

  goDistbaseLeft1SetDist = setDist;
  goDistbaseLeft2SetDist = setDist;
  goDistbaseRight1SetDist = setDist;
  goDistbaseRight2SetDist = setDist;

  while ((t < time) && (e < errCutTime)) {
    goDistbaseLeft1Current = (int) left_mtr.get_position();
    goDistbaseLeft2Current = (int) left_mtr2.get_position();
    goDistbaseRight1Current = (int) right_mtr.get_position();
    goDistbaseRight2Current = (int) right_mtr2.get_position();
    goDistbaseLeft1Error = goDistbaseLeft1SetDist - goDistbaseLeft1Current;
    goDistbaseLeft2Error = goDistbaseLeft2SetDist - goDistbaseLeft2Current;
    goDistbaseRight1Error = goDistbaseRight1SetDist - goDistbaseRight1Current;
    goDistbaseRight2Error = goDistbaseRight2SetDist - goDistbaseRight2Current;
    left_mtr = maxAutonSpeed(maxSpeed, goDistbaseLeft1Error * goDistKp);
    left_mtr2 = maxAutonSpeed(maxSpeed, goDistbaseLeft2Error * goDistKp);
    right_mtr = maxAutonSpeed(maxSpeed, goDistbaseRight1Error * goDistKp);
    right_mtr2 = maxAutonSpeed(maxSpeed, goDistbaseRight2Error * goDistKp);
    /*
    left_mtr = goDistbaseLeft1Error * goDistKp;
    left_mtr2 = goDistbaseLeft2Error * goDistKp;
    right_mtr = goDistbaseRight1Error * goDistKp;
    right_mtr2 = goDistbaseRight2Error * goDistKp;
    */
    pros::delay(1);
    t += 1;
    if ((((goDistbaseLeft1Error < 20) && (goDistbaseLeft1Error > 0)) || ((goDistbaseLeft1Error > -20) && (goDistbaseLeft1Error < 0))) || (goDistbaseLeft1Error == 0)) {
      pros::delay(1);
      e += 1;
    }
    if (printPIDBasedAutonVal1) {
      pros::lcd::print(1, "L1Curr: %d, L1SetDist: %d", goDistbaseLeft1Current, goDistbaseLeft1SetDist);
      pros::lcd::print(2, "t: %d, e: %d", t, e);
    }
  }
  if (printPIDBasedAutonVal1) {
    pros::lcd::set_text(3, "exited while loop");
  }
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(100);
}

void turnFor (double setDist, double setKp, int time, int errCutTime, int maxSpeed = 80) {
  double goDistKp = setKp;
  int t = 0;
  int e = 0;

  //sets the absolute zero on the motor encoder to current value
  left_mtr.tare_position();
  left_mtr2.tare_position();
  right_mtr.tare_position();
  right_mtr2.tare_position();

  goDistbaseLeft1SetDist = -setDist;
  goDistbaseLeft2SetDist = -setDist;
  goDistbaseRight1SetDist = setDist;
  goDistbaseRight2SetDist = setDist;

  while ((t < time) && (e < errCutTime)) {
    goDistbaseLeft1Current = (int) left_mtr.get_position();
    goDistbaseLeft2Current = (int) left_mtr2.get_position();
    goDistbaseRight1Current = (int) right_mtr.get_position();
    goDistbaseRight2Current = (int) right_mtr2.get_position();
    goDistbaseLeft1Error = goDistbaseLeft1SetDist - goDistbaseLeft1Current;
    goDistbaseLeft2Error = goDistbaseLeft2SetDist - goDistbaseLeft2Current;
    goDistbaseRight1Error = goDistbaseRight1SetDist - goDistbaseRight1Current;
    goDistbaseRight2Error = goDistbaseRight2SetDist - goDistbaseRight2Current;
    left_mtr = maxAutonSpeed(maxSpeed, goDistbaseLeft1Error * goDistKp);
    left_mtr2 = maxAutonSpeed(maxSpeed, goDistbaseLeft2Error * goDistKp);
    right_mtr = maxAutonSpeed(maxSpeed, goDistbaseRight1Error * goDistKp);
    right_mtr2 = maxAutonSpeed(maxSpeed, goDistbaseRight2Error * goDistKp);
    /*
    left_mtr = goDistbaseLeft1Error * goDistKp;
    left_mtr2 = goDistbaseLeft2Error * goDistKp;
    right_mtr = goDistbaseRight1Error * goDistKp;
    right_mtr2 = goDistbaseRight2Error * goDistKp;
    */
    pros::delay(1);
    t += 1;
    if ((((goDistbaseLeft1Error < 20) && (goDistbaseLeft1Error > 0)) || ((goDistbaseLeft1Error > -20) && (goDistbaseLeft1Error < 0))) || (goDistbaseLeft1Error == 0)) {
      pros::delay(1);
      e += 1;
    }
    if (printPIDBasedAutonVal1) {
      pros::lcd::print(1, "L1Curr: %d, L1SetDist: %d", goDistbaseLeft1Current, goDistbaseLeft1SetDist);
      pros::lcd::print(2, "t: %d, e: %d", t, e);
    }
  }
  if (printPIDBasedAutonVal1) {
    pros::lcd::set_text(3, "exited while loop");
  }
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(100);
}


///////////////////////////////////////////////////////////////////////////////////////////////////////

void on_center_button() {
	static bool pressed = false;
	pressed = !pressed;
	if (pressed) {
		pros::lcd::set_text(2, "I was pressed!");
	} else {
		pros::lcd::clear_line(2);
	}
}
/**
 * Runs initialization code. This occurs as soon as the program is started.
 *
 * All other competition modes are blocked by initialize; it is recommended
 * to keep execution time for this mode under a few seconds.
 */
void initialize() {
	pros::lcd::initialize();
	//pros::lcd::set_text(1, "Hello PROS User!");

	pros::lcd::register_btn1_cb(on_center_button);
  //pros::Task updatePos (updateRobotPos, (void*)"", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Update Robot Position");
  //the tray task is initialized here because it'll be used in auton
  pros::Task tray (updateTrayPID, (void*)"", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Update PID for the tray motor");
  pros::Task roller (updateRollerPID, (void*)"", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Update PID for the roller motors");
  pros::Task lift (updateLiftPID, (void*)"", TASK_PRIORITY_DEFAULT, TASK_STACK_DEPTH_DEFAULT, "Update PID for the lift motor");

}
/**
 * Runs while the robot is in the disabled state of Field Management System or
 * the VEX Competition Switch, following either autonomous or opcontrol. When
 * the robot is enabled, this task will exit.
 */
void disabled() {}
/**
 * Runs after initialize(), and before autonomous when connected to the Field
 * Management System or the VEX Competition Switch. This is intended for
 * competition-specific initialization routines, such as an autonomous selector
 * on the LCD.
 *
 * This task will exit when the robot is enabled and autonomous or opcontrol
 * starts.
 */
void competition_initialize() {}
/**
 * Runs the user autonomous code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the autonomous
 * mode. Alternatively, this function may be called in initialize or opcontrol
 * for non-competition testing purposes.
 *
 * If the robot is disabled or communications is lost, the autonomous task
 * will be stopped. Re-enabling the robot will restart the task, not re-start it
 * from where it left off.
 */
void autonomous() {
  //autonLift: 0 = null, 1 = flip out up PID, 2 = flip out PID at 0
  //autonTray: 0 = null, 1 = auto stack sequence, 2 = flip out up PID, 3 = flip out PID at 0
  /*
  autonTray = 2;
  pros::delay(1000);
  autonLift = 1;
  pros::delay(1000);
  autonLift = 2;
  pros::delay(1000);
  autonTray = 3;
  pros::delay(1000);
  */

  /*
  //blue small side 5 cube
  autonRoller = 2; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold, 4 = outtake at 65 speed, 5 = outtake at 30 speed
  goDistance (1200, 1.5, 6000, 200, 43); //go 1000 ticks forward with 1.5 kP for 10000 frames, if value reached stop after 6000 frames. max speed is 80
  goDistance (-450, 1.5, 1500, 200, 80);
  turnFor (400, 1.5, 1000, 200, 80); //positive ticks turns left, negative turns right, same arguments as goDistance()
  goDistance (580, 1.5, 1000, 200, 80);
  autonRoller = 4;
  pros::delay(400);
  autonTray = 1; //0 = null, 1 = tray PID stacking sequence
  autonRoller = 5;
  pros::delay(1000);
  autonRoller = 3;
  pros::delay(1000);
  autonRoller = 2;
  pros::delay(500);
  autonRoller = 1;
  left_mtr = -100;
  left_mtr2 = -100;
  right_mtr = -100;
  right_mtr2 = -100;
  pros::delay(200);
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(200);
  */
  /*
  //blue small side 6 cube
  autonRoller = 2; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold, 4 = outtake at 65 speed, 5 = outtake at 30 speed
  goDistance (1200, 1.5, 6000, 200, 43); //go 1000 ticks forward with 1.5 kP for 10000 frames, if value reached stop after 6000 frames. max speed is 80
  turnFor (-59, 1.5, 1000, 200, 80);
  goDistance (200, 1.5, 1000, 200, 90);
  goDistance (-200, 1.5, 1000, 200, 90);
  turnFor (59, 1.5, 1000, 200, 80);
  goDistance (-450, 1.5, 1500, 200, 80);
  turnFor (400, 1.5, 1000, 200, 80); //positive ticks turns left, negative turns right, same arguments as goDistance()
  goDistance (580, 1.5, 1000, 200, 80);
  autonRoller = 4;
  pros::delay(400);
  autonTray = 1; //0 = null, 1 = tray PID stacking sequence
  autonRoller = 5;
  pros::delay(1000);
  autonRoller = 3;
  pros::delay(1000);
  autonRoller = 2;
  pros::delay(500);
  autonRoller = 1;
  left_mtr = -100;
  left_mtr2 = -100;
  right_mtr = -100;
  right_mtr2 = -100;
  pros::delay(200);
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(200);
  */
  /*
  //red small side 6 cube
  autonRoller = 2; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold, 4 = outtake at 65 speed, 5 = outtake at 30 speed
  goDistance (1200, 1.5, 6000, 200, 43); //go 1000 ticks forward with 1.5 kP for 10000 frames, if value reached stop after 6000 frames. max speed is 80
  turnFor (59, 1.5, 1000, 200, 80);
  goDistance (200, 1.5, 1000, 200, 90);
  goDistance (-200, 1.5, 1000, 200, 90);
  turnFor (-59, 1.5, 1000, 200, 80);
  goDistance (-450, 1.5, 1500, 200, 80);
  turnFor (-400, 1.5, 1000, 200, 80); //positive ticks turns left, negative turns right, same arguments as goDistance()
  goDistance (580, 1.5, 1000, 200, 80);
  autonRoller = 4;
  pros::delay(400);
  autonTray = 1; //0 = null, 1 = tray PID stacking sequence
  autonRoller = 5;
  pros::delay(1000);
  autonRoller = 3;
  pros::delay(1000);
  autonRoller = 2;
  pros::delay(500);
  autonRoller = 1;
  left_mtr = -100;
  left_mtr2 = -100;
  right_mtr = -100;
  right_mtr2 = -100;
  pros::delay(200);
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(200);
  */
  /*
  //red small side 8 cube
  autonRoller = 2; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold, 4 = outtake at 65 speed, 5 = outtake at 30 speed
  goDistance (1200, 1.5, 6000, 200, 43); //go 1000 ticks forward with 1.5 kP for 10000 frames, if value reached stop after 6000 frames. max speed is 80
  goDistance (-450, 1.5, 1500, 200, 80);
  turnFor (59, 1.5, 1000, 200, 80);
  goDistance (-500, 1.5, 1500, 200, 100);
  goDistance (1000, 1.5, 6000, 200, 43);
  turnFor (59, 1.5, 1000, 200, 80);
  goDistance (200, 1.5, 1000, 200, 90);
  goDistance (-200, 1.5, 1000, 200, 90);
  turnFor (-59, 1.5, 1000, 200, 80);
  goDistance (-450, 1.5, 1500, 200, 80);
  turnFor (-400, 1.5, 1000, 200, 80); //positive ticks turns left, negative turns right, same arguments as goDistance()
  goDistance (580, 1.5, 1000, 200, 80);
  autonRoller = 4;
  pros::delay(400);
  autonTray = 1; //0 = null, 1 = tray PID stacking sequence
  autonRoller = 5;
  pros::delay(1000);
  autonRoller = 3;
  pros::delay(1000);
  autonRoller = 2;
  pros::delay(500);
  autonRoller = 1;
  left_mtr = -100;
  left_mtr2 = -100;
  right_mtr = -100;
  right_mtr2 = -100;
  pros::delay(200);
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(200);
  */


  /*
  //red small side 8 cube (working)
  autonTrayKp = 0.17;
  autonRoller = 2; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold, 4 = outtake at 65 speed, 5 = outtake at 30 speed
  goDistance (1200, 1.0, 6000, 100, 70); //go 1000 ticks forward with 1.5 kP for 10000 frames, if value reached stop after 6000 frames. max speed is 80
  goDistance (-450, 0.85, 1500, 100, 127);
  turnFor (150, 1.2, 1000, 100, 80);
  goDistance (-750, 1.0, 1500, 100, 127);
  turnFor (-165, 1.2, 1000, 100, 80);
  goDistance (950, 1.0, 6000, 300, 50);
  autonRoller = 3;
  goDistance (-600, 0.8, 5000, 150, 110);
  turnFor (-390, 2.0, 5000, 150, 55); //positive ticks turns left, negative turns right, same arguments as goDistance()
  goDistance (650, 1.5, 1000, 100, 80);
  autonTray = 1; //0 = null, 1 = tray PID stacking sequence
  autonRoller = 3;
  pros::delay(1000);
  autonRoller = 2;
  pros::delay(400);
  autonRoller = 1;
  left_mtr = -100;
  left_mtr2 = -100;
  right_mtr = -100;
  right_mtr2 = -100;
  pros::delay(200);
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(200);
  */

  //blue small side 8 cube (working)
  autonTrayKp = 0.17;
  autonRoller = 2; //0 = null, 1 = outtake, 2 = intake, 3 = roller PID Hold, 4 = outtake at 65 speed, 5 = outtake at 30 speed
  goDistance (1300, 1.0, 6000, 100, 70); //go 1000 ticks forward with 1.5 kP for 10000 frames, if value reached stop after 6000 frames. max speed is 80
  goDistance (-550, 0.85, 1500, 100, 127);
  turnFor (-150, 1.2, 1000, 100, 80);
  goDistance (-750, 1.0, 1500, 100, 127);
  turnFor (160, 1.2, 1000, 100, 80);
  goDistance (1050, 1.0, 6000, 300, 50);
  goDistance (-600, 0.8, 5000, 150, 110);
  autonRoller = 5;
  turnFor (340, 2.0, 5000, 150, 55); //positive ticks turns left, negative turns right, same arguments as goDistance() Shishir reduced by 10 ticks
  autonRoller = 3;
  goDistance (650, 1.5, 1000, 100, 80);
  autonTray = 1; //0 = null, 1 = tray PID stacking sequence
  autonRoller = 3;
  pros::delay(1000);
  autonRoller = 2;
  pros::delay(400);
  autonRoller = 1;
  left_mtr = -100;
  left_mtr2 = -100;
  right_mtr = -100;
  right_mtr2 = -100;
  pros::delay(200);
  left_mtr = 0;
  left_mtr2 = 0;
  right_mtr = 0;
  right_mtr2 = 0;
  pros::delay(200);
}
/**
 * Runs the operator control code. This function will be started in its own task
 * with the default priority and stack size whenever the robot is enabled via
 * the Field Management System or the VEX Competition Switch in the operator
 * control mode.
 *
 * If no competition control is connected, this function will run immediately
 * following initialize().
 *
 * If the robot is disabled or communications is lost, the
 * operator control task will be stopped. Re-enabling the robot will restart the
 * task, not resume it from where it left off.
 */
void opcontrol() {



  autonTray = 0;
  autonRoller = 0;
  autonTrayKp = 0.10;

	while (true) {

    //autonTray = false;
    //autonRoller = 0;
		// pros::lcd::print(0, "%d %d %d", (pros::lcd::read_buttons() & LCD_BTN_LEFT) >> 2,
		//                  (pros::lcd::read_buttons() & LCD_BTN_CENTER) >> 1,
		//                  (pros::lcd::read_buttons() & LCD_BTN_RIGHT) >> 0);

///////////////////////////////////////////////////////////////////////////////////////////////////////

	 	//left base true speed
	 	left_mtr = returnTrueSpeed (master.get_analog(ANALOG_LEFT_Y));
		left_mtr2 = returnTrueSpeed (master.get_analog(ANALOG_LEFT_Y));
	 	//right base true speed
	  right_mtr = returnTrueSpeed (master.get_analog(ANALOG_RIGHT_Y));
		right_mtr2 = returnTrueSpeed (master.get_analog(ANALOG_RIGHT_Y));


    /*
    //left base true speed
	 	left_mtr = returnTrueSpeed (master.get_analog(ANALOG_LEFT_Y)) + returnTrueSpeed (master.get_analog(ANALOG_RIGHT_X))/2;
		left_mtr2 = returnTrueSpeed (master.get_analog(ANALOG_LEFT_Y)) + returnTrueSpeed (master.get_analog(ANALOG_RIGHT_X))/2;
	 	//right base true speed
	  right_mtr = returnTrueSpeed (master.get_analog(ANALOG_LEFT_Y)) - returnTrueSpeed (master.get_analog(ANALOG_RIGHT_X))/2;
		right_mtr2 = returnTrueSpeed (master.get_analog(ANALOG_LEFT_Y)) - returnTrueSpeed (master.get_analog(ANALOG_RIGHT_X))/2;
    */

	 	if (printBaseVals){
	     	pros::lcd::print(3, "left_mtr: %d", left_mtr);
				pros::lcd::print(4, "left_mtr2: %d", left_mtr2);
				pros::lcd::print(5, "right_mtr: %d", right_mtr);
				pros::lcd::print(6, "right_mtr2: %d", right_mtr2);
	 	}

///////////////////////////////////////////////////////////////////////////////////////////////////////

    if (master.get_digital(DIGITAL_DOWN)) {
      //shortestTurn (130*M_PI/180);
    }


    //pros::lcd::print(2, "simpleCurrA: %f", makeAngleSimple(currA));

		pros::delay(10);
	}
}
