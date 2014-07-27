// NEURAL NET FOR LEFT 3

#include "mbed.h"
#include "MX28/MX28.h"
#include <cmath>

#define PI 3.14159265


// Posterior and Anterior extreme positions for the ThC joint.
// Note: FOR CONTRALATTERAL LEGS SWAP tcPEP and tcAEP
#define tcPEP 0xA28
#define tcAEP 0x762 

// FTi joint stance and swing goal angle positions.
#define ft_StanceP 0x0890  
#define ft_SwingP 0x7DA   

// CTr joint highest and lowest goal angle positions.
#define ct_highP 0x0890     
#define ct_lowP 0x0Af0        
  
#define des_Height 14      // Desired Robot Height 

// Global Variables for the coxa,femur,tibia length
// Also for the offset angle in readians.
static const double coxa = 5;
static const double femur =22.5;
static const double tibia =20.5;
static const double offset = (40*PI)/180;

// The pins used for the serial communication.
MX28 mx28(p9,p10,3000000);

DigitalOut startStopLED(LED1);
DigitalOut stepLED(LED3);
DigitalOut stanceLED(LED4);
// Touch Sensor input.
DigitalIn pb(p30);
static const int updateInterval = 100000;       //Step size.......
static const int ctrlInterval = 10000;          //Servo control time interval...100Hz
Timer stepTimer;
Timer ctrlTimer;
Serial pc(USBTX,USBRX);
bool isWalking = false;

enum Commands
{ 
    Start = 0x00,
    Stop = 0x01    
};

//**********************************************
////////////////////////////////////////////////
////////////////////////////////////////////////
//*******************JOINTs ********************
// The Joint class represents the actions that a joint can do.
class Joint

{
public:
    void SetId(uint8_t id);
    uint16_t GetAngle();
    uint16_t GetVelocity();
    void Pro(double speed);
    void Ret(double speed);
    void Lev(double speed);
    void Dep(double speed);
    void Flx(double speed);
    void Ext(double speed);
private:
    uint8_t jointId;
    uint16_t jointAngle;
    uint16_t jointVelocity;
};

void Joint::Flx(double speed)
{
     if(jointId % 3 == 0){
        uint16_t mvspeed = (uint16_t)speed; 
        mx28.SetMovingSpeed(jointId,mvspeed);
        mx28.SetGoalPosition(jointId,ft_StanceP);
    }
}

void Joint::Ext(double speed)
{
    if(jointId % 3 == 0){
        uint16_t mvspeed = (uint16_t)speed;
        mx28.SetMovingSpeed(jointId,mvspeed);
        mx28.SetGoalPosition(jointId,ft_SwingP);
    }
}

void Joint::Dep(double speed)
{
    if(jointId % 3 == 2){
        uint16_t mvspeed = (uint16_t)speed;
        mx28.SetMovingSpeed(jointId,mvspeed);
        mx28.SetGoalPosition(jointId,ct_lowP);
    }
}

void Joint::Lev(double speed)
{
    if(jointId % 3 == 2){
        uint16_t mvspeed = (uint16_t)speed;
        mx28.SetMovingSpeed(jointId,mvspeed);
        mx28.SetGoalPosition(jointId,ct_highP);
    }
}



void Joint::Ret(double speed)
{
    if(jointId % 3 == 1){
        uint16_t mvspeed = (uint16_t)speed;
        mx28.SetMovingSpeed(jointId,mvspeed);
        mx28.SetGoalPosition(jointId,tcPEP);
    }
}

void Joint::Pro(double speed)
{
    if(jointId % 3 == 1){
        uint16_t mvspeed = (uint16_t)speed;
        mx28.SetMovingSpeed(jointId,mvspeed);
        mx28.SetGoalPosition(jointId,tcAEP);
    }
}

uint16_t Joint::GetVelocity()
{
    mx28.GetPresentSpeed(jointId,&jointVelocity);
    return jointVelocity;
}

uint16_t Joint::GetAngle()
{
    mx28.GetPresentPosition(jointId,&jointAngle);
    return jointAngle;
}

void Joint::SetId(uint8_t id)
{
    Joint::jointId = id;
    switch (id % 3){
        // ThC
		case 1:
            mx28.SetCWAngleLimit(id,0x78A); // 1930 // 170 // real angle is -10
            mx28.SetCCWAngleLimit(id,0xAAA); // 2730  // 240 // real angle is 60 
            break;
		// CTr	
        case 2:
            mx28.SetCWAngleLimit(id,0x0708); // 1800 // 160 // real angle is 110
            mx28.SetCCWAngleLimit(id,0xCE4); // 3300 // 290 // real angle is -20
            break;
		// FTi
        case 0:
            mx28.SetCWAngleLimit(id,0x04E2);  // 1250 // 110 // real angle is 20
            mx28.SetCCWAngleLimit(id,0x0988); // 2440 // 215 // real angle is 125
            break;
    }
}

Joint jointTC;                                     
Joint jointCT;
Joint jointFT;

//*********************************************************
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//****************Sigmoid&MotorServo Neuron****************
// The Hidden Layer of the Neural Network.
class Hidden                                       
{
public:
	
	Hidden(double selfweight,double bias);
    //**********Get Outputs From Sigmoid Neuron and Push to Sigmoid Layer***********//
    double Push_s(double input1,double input2,double weight1,double weight2);
    //********** "OR" Neuron **********//
    double Push_or(double input1, double input2, double weight1, double weight2);
    //********** "AND" Neuron **********//
    double Push_and(double input1, double input2, double weight1, double weight2);
	
	//********** Sigmoid neuron function for angle inputs **********//
	double Push_s_sensor(double input_angle, double angle_weight,double SN_Thresh);

private:
    double selfWeight;
    double bias;
};

double Hidden::Push_and(double input1, double input2, double weight1, double weight2)
{
    if(input1*weight1+bias > 0 && input2*weight2+bias > 0){
        return 1.0;
    } else {
		return 0.0;
	}
}

double Hidden::Push_or(double input1, double input2, double weight1, double weight2)
{
    if(input1*weight1+bias > 0 || input2*weight2+bias > 0){
        return 1.0;
    } else {
		return 0.0;
	}
}
double Hidden :: Push_s_sensor(double input1,double weight1, double SN_Thresh)
{
	
	double bias_adj = 3.2;
	double TN_bias = -(weight1*SN_Thresh+0.5*selfWeight)+bias_adj;
	double input = -(input1*weight1+selfWeight+TN_bias);
	double result = 1/(1+std::exp(input));          //SIGMOID function 
    return result;
}
double Hidden::Push_s(double input1,double input2,double weight1,double weight2)
{
    
	double input = -(input1*weight1+input2*weight2+selfWeight+bias);
    double result = 1/(1+std::exp(input));          //SIGMOID function 
    return result;
}

Hidden::Hidden(double selfweight,double bias)
{
    Hidden::selfWeight = selfweight;
    Hidden::bias = bias;
}

// ThC
Hidden sigmoid17(5,-17.5);   

// CTr
Hidden sigmoid18(5,8.70);
Hidden sigmoid19(5,13.148);
Hidden orNeuron20(0,-0.6);
Hidden sigmoid21(5,-16.9);
Hidden sigmoid22(5,-17.96);
Hidden orNeuron23(0,-25);
Hidden sigmoid15(5,-2.00);

// FTi
Hidden sigmoid27(5,-16);
Hidden sigmoid28(5,-16);
Hidden orNeuron29(0,-17);
Hidden sigmoid30(5,16);
Hidden sigmoid31(5,16);
Hidden andNeuron32(0,-18);
Hidden sigmoid16(5,8);

//******************************************************************
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//*******************SERVO MODULE********************************
class Servo
{
public:
    Servo(double bias);
    double ThCServo(double tcAngle, double tcVelocity, double output17);
    double CTrHeightServo(double femurH, double tibiaH, double ctAngle, double cta_w);
	void FTiServo();
private:
    double bias;
};

double Servo::CTrHeightServo(double ftAngle, double ctAngle, double tcAngle, double cta_w)
{


	
	double ftRad = PI*ftAngle/180;
	double tcRad = PI*abs(tcAngle)/180;
	
    double C = ((des_Height/cos(offset))/cos(tcRad)-cos(offset)*coxa);
    double B = cos(ftRad+offset)*tibia+cos(offset)*femur;
    double A = sin(ftRad+offset)*tibia+sin(offset)*femur;
    double X = sqrt((C*C+B*B)/(B*B+A*A));
    double des_ctAngle = acos(X)*180/PI;
	

	double difference = des_ctAngle-ctAngle;

	if (difference >=0)
		return 1;
	else 
		return -1;

}

double Servo::ThCServo(double tcAngle, double tcVelocity, double output17)
{

	double des_p = 0;
    if(tcVelocity > 1023){
        tcVelocity = tcVelocity - 1023;
    }
    else{
        tcVelocity = -tcVelocity;
    }
    double input2 = tcVelocity;
    if(output17 >= 0.5){
        des_p  = ((double)tcPEP - 2048)*0.088;
    } else {
		des_p  = ((double)tcAEP - 2048)*0.088;
    }
	double output = des_p*(-32) + tcAngle*(-33)+bias ;
	double result = 1/(1+std::exp(-output));

	return result;
	
}

Servo::Servo(double bias)
{
    Servo::bias = bias;
}

Servo servo14(-22);
Servo servo24(des_Height);
//******************************************************************
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//*******************BISTABLE MODULE********************************
class Bimodule
{
public:
    Bimodule(double bias);
    void tcJointCtrl(double input, double weight, double speed);
	void ctJointCtrl(double input,double weight, double speed);
    void ftJointCtrl(double input, double weight, double speed);
	
private:
    double bias;
};


void Bimodule::ftJointCtrl(double input, double weight, double speed)
{
    double output = input*weight + bias; 
	

	if (output>0 && input>=0.5){
		jointFT.Flx(speed);
		}
	if (output>0 && input<0.5) {
		jointFT.Ext(speed);
	}
	
}

void Bimodule::ctJointCtrl(double input, double weight, double speed)
{
    double output = input*weight + bias;
	
	if (output >0&&input>=0.5){
		
		jointCT.Lev(speed);
}		
	if (output >0&&input<0.5){
	
		jointCT.Dep(speed);	
	}
	
}

void Bimodule::tcJointCtrl(double input, double weight, double speed)
{
    double output = input*weight + bias;

	if (output >0 && input>=0.5){

		jointTC.Pro(speed);}
	if (output > 0 && input<0.5){

		jointTC.Ret(speed);}
    
}

Bimodule::Bimodule(double bias)
{
    Bimodule::bias = bias;
}

// BIAS VALUES
// neuron 8
Bimodule Protraction(-16);
// neuron 9
Bimodule Retraction(16);
// neuron 10
Bimodule Levation(-16);
// neuron 11
Bimodule Depression(16);
// neuron 12
Bimodule Flexion(-16);
// neuron 13
Bimodule Extension(16);

//******************************************************************
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//*******************CONTROL NETWORK********************************
class Control
{
public:
    
    Control(uint8_t id1, uint8_t id2, uint8_t id3);
    //Control Network
    void Control_all(uint16_t speed);
private:
    double output17;
    double output14;
    
    double output18;
    double output19;
	double output20;
    double output21;
    double output22;
	double output23;
	double output15;
	double output24;
	
	double output27;
	double output28;
	double output29;
	double output30;
	double output31;
	double output32;
    double output16;
};

void Control::Control_all(uint16_t speed)
{
    ctrlTimer.reset();
    ctrlTimer.start();
    int handler = 10 ;
    while(isWalking&&(handler>0))
    {
        int timerValue = ctrlTimer.read_us();
        if(timerValue >= ctrlInterval){
            //.....................Read Inputs..................
            pb.mode(PullUp);
            double step_speed = (unsigned)speed;
            double tcAngle = (unsigned)jointTC.GetAngle();
            double tcVel = (unsigned)jointTC.GetVelocity();
            double ctAngle = (unsigned)jointCT.GetAngle();
            double ftAngle = (unsigned)(jointFT.GetAngle()-0x0400);
			double f_Contact = !pb?1.0:0.0;
           
			// Adjusting the angles to be the same as in Von Twickel's implementation.
			tcAngle = tcAngle - 2048;
			tcAngle = -tcAngle *0.088;
			ctAngle = 270-ctAngle*0.088;
			ftAngle = ftAngle*0.088;
			
			//.....................ThC Joint Control............//
            // 7->17
            output17 = sigmoid17.Push_s_sensor(f_Contact,20,0.5);                          
			// 1,2,17->14
            output14 = servo14.ThCServo(tcAngle,tcVel,output17);
			
			// 14->8
			Protraction.tcJointCtrl(output14,32,step_speed);
            // 14->9
            Retraction.tcJointCtrl(output14,-32,step_speed);  
			
			
            //.....................CTr Joint Control............//
			double dep_to_lev_ft = ftAngle - 55; 
			double dep_to_lev_tc = tcAngle - (-45);
			double lev_to_dep_tc = tcAngle - 5;
            double lev_to_dep_ft = ftAngle - 90;
			
			// 1->18
			output18 = sigmoid18.Push_s_sensor(dep_to_lev_tc,-32,0.250);
			// 5->19
            output19 = sigmoid19.Push_s_sensor(dep_to_lev_ft,-32,0.389);   
			// 5->21
			output21 = sigmoid21.Push_s_sensor(lev_to_dep_tc,32,0.528);
			// 5->22
            output22 = sigmoid22.Push_s_sensor(lev_to_dep_ft,32,0.583);
			// 18,19 -> 20
			output20 = orNeuron20.Push_or(output18,output19,32,32);		
			//21,22->23
			output23 = orNeuron23.Push_or(output21,output22,32,32);		
			// 20,23->15
            output15 = sigmoid15.Push_s(output20,output23,30,-31);  
			
			// HEIGHT CONTROLLER
			output24 = servo24.CTrHeightServo(ftAngle,ctAngle,tcAngle,-4.2);
			if (f_Contact>0.5){
				if (output24>0){
					Levation.ctJointCtrl(output24,32,step_speed);
				} else {
					Depression.ctJointCtrl(output24,32,step_speed);
				}
			}
			
			// 15->10
            Levation.ctJointCtrl(output15,32,step_speed);
            // 15->11
            Depression.ctJointCtrl(output15,-32,step_speed);
			
			
            //.....................FTi Joint Control............

			output27 = sigmoid27.Push_s(ftAngle,0,32,0);
			output28 = sigmoid28.Push_s(0,f_Contact,0,20);
			output29 = orNeuron29.Push_or(output27,output28,32,32);
			output30 = sigmoid30.Push_s(ftAngle,0,-32,0);
			output31 = sigmoid31.Push_s(0,f_Contact,0,-20);
			output32 = andNeuron32.Push_and(output31,output32,32,32);
			output16 = sigmoid16.Push_s(output29,output32,-28,32);
			
			Flexion.ftJointCtrl(output16,32,step_speed);
			Extension.ftJointCtrl(output16,-32,step_speed);
			
			ctrlTimer.reset();
            handler = handler - 1;
                
        }
        
    }
}

Control::Control(uint8_t id1, uint8_t id2, uint8_t id3)
{
    jointTC.SetId(id1);
    jointCT.SetId(id2);
    jointFT.SetId(id3);

}
//**********************************************
////////////////////////////////////////////////
////////////////////////////////////////////////
//*******************MAIN FUNCTION**************
int main()
{
    pc.baud(115200);
 
	Control control(0x07, 0x08, 0x09);
    while(true)
    {
        if(pc.readable()){
            uint8_t command = pc.getc();
            switch(command)
            {
                case '1':                    
                    stepTimer.reset();
                    stepTimer.start();
                    isWalking = true;
                    startStopLED = 1;
                    break;
                case '2':                    
                    stepTimer.stop();
                    isWalking = false;
                    startStopLED = 0;
                    stepLED = 0;
                    mx28.SetEnableTorque(0x07,0);
                    mx28.SetEnableTorque(0x08,0);
                    mx28.SetEnableTorque(0x09,0);
                    break;
            }
        }
	
        if(isWalking){
            int timerValue = stepTimer.read_us();
            if(timerValue > updateInterval){
                stepLED = !stepLED;
				control.Control_all(0x0020);
                stepTimer.reset();
            }
        }
        
    }
}
