// NEURAL NET FOR LEFT 2

#include "mbed.h"
#include "MX28/MX28.h"
#include <cmath>

#define PI 3.14159265


// Posterior and Anterior extreme positions for the ThC joint.
// Note: FOR CONTRALATTERAL LEGS SWAP tcPEP and tcAEP
#define tcPEP 0x09c4 
#define tcAEP 0x0640 

// FTi joint stance and swing goal angle positions.
#define ft_StanceP 0x0890  
#define ft_SwingP 0x06A4   

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
            mx28.SetCWAngleLimit(id,0x05C8); // 1480 // 130 // real angle is -50
            mx28.SetCCWAngleLimit(id,0x09c4); // 2500 // 220 // real angle is 40 
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
Hidden sigmoid17(5,-17.5); // no  

// CTr
Hidden sigmoid18(5,12.26); // no

Hidden sigmoid19(5,-23.3); // no
Hidden orNeuron20(0,-17); // Yes
Hidden sigmoid21(5,15.81); // no 
Hidden sigmoid15(5,-5); // yes

// FTi
Hidden sigmoid25(5,-20.26); // no
Hidden sigmoid26(5,12.5); // yes
Hidden orNeuron27(0,-5); // yes
Hidden sigmoid28(5,15.26); // no
Hidden sigmoid29(5,-17.5); // yes
Hidden andNeuron30(0,-15); // yes
Hidden sigmoid16(16,-10);// yes


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
    double output15;
    
	double output24;
    double output25;
    double output26;
	double output27;
    double output28;
    double output29;
	double output30;
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
			double dep_to_lev_ft = ftAngle-120; 
			double dep_to_lev_tc = tcAngle -(-25);
            double lev_to_dep_ft = ftAngle-70;
			
			// 1->18
			output18 = sigmoid18.Push_s_sensor(dep_to_lev_tc,-32,0.361);
			// 5->19
            output19 = sigmoid19.Push_s_sensor(dep_to_lev_ft,32,0.750);   
			// 5->21
            output21 = sigmoid21.Push_s_sensor(lev_to_dep_ft,-32,0.472);
			// 18,19 -> 20
			output20 = orNeuron20.Push_or(output18,output19,32,32);		
			// 20,21->15
            output15 = sigmoid15.Push_s(output20,output21,32,-29);                 
           
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
            
			double flx_to_ext_ft = ftAngle - 105;
			double ext_to_flx_ft = ftAngle - 105;
			
			//5->25
            output25 = sigmoid25.Push_s_sensor(flx_to_ext_ft,32,0.667);
			//7->26
            output26 = sigmoid26.Push_s(0,f_Contact,0,-20);
			//25,26->27
            output27 = orNeuron27.Push_or(output25,output26,32,32);
			//5->28
            output28 = sigmoid28.Push_s_sensor(ext_to_flx_ft,-32,0.667);
			// 7->29
            output29 = sigmoid29.Push_s(0,f_Contact,0,20);
			//28,29->30
            output30 = andNeuron30.Push_and(output28,output29,32,32);
            // 27,30->16
            output16 = sigmoid16.Push_s(output27,output30,-30,32);                     
			// 16->12
			Flexion.ftJointCtrl(output16,32,step_speed);
            // 16->13
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