// NEURAL NET FOR LEFT 1


#include "mbed.h"
#include "MX28/MX28.h"
#include <cmath>

#define PI 3.14159265
#define tcPEP 0x09c0
#define tcAEP 0x0640
#define ft_mStanceP 0x0890
#define ft_SwingP 0x0730
#define ct_highP 0x0770
#define ct_lowP 0x09c8
#define des_Height 14.0
MX28 mx28(p13,p14,3000000);
DigitalOut startStopLED(LED1);
DigitalOut stepLED(LED3);
DigitalOut stanceLED(LED4);
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
class Joint

// Set and Get method constructors.
{
public:
    void SetId(uint8_t id);
    uint16_t GetAngle();
    uint16_t GetVelocity();
    void Pro(double speed);
    void Ret(double speed);
    void InStepLev(double input);
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
        mx28.SetGoalPosition(jointId,ft_mStanceP);
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

void Joint::InStepLev(double input)
{
    if(jointId % 3 == 2){
        mx28.SetMovingSpeed(jointId,0x0010);
        uint16_t des_p = (uint16_t)input;
        mx28.SetGoalPosition(jointId,des_p);
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
        case 1:
            mx28.SetCWAngleLimit(id,0x0400); // 1024 // 90
            mx28.SetCCWAngleLimit(id,0x0c00); // 3072 // 270 
            break;
        case 2:
            mx28.SetCWAngleLimit(id,0x07f0); // 2032 // 180
            mx28.SetCCWAngleLimit(id,0x09f0); // 2544 // 224
            break;
        case 0:
            mx28.SetCWAngleLimit(id,0x0400);  // 1024 // 90
            mx28.SetCCWAngleLimit(id,0x0980); // 2432 // 214
            break;
    }
}

Joint jointTC;                                      //All neurons and joints are global variables
Joint jointCT;
Joint jointFT;

//*********************************************************
///////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////
//****************Sigmoid&MotorServo Neuron****************
class Hidden                                       //Act as hidden layers
{
public:
    
    Hidden(double selfweight,double bias);
    //**********Get Outputs From Sigmoid Neuron and Push to Sigmoid Layer***********//
    double Push_s(double input1,double input2,double weight1,double weight2);
    //********** "OR" Neuron **********//
    double Push_or(double input1, double input2, double weight1, double weight2);
    //********** "AND" Neuron **********//
    double Push_and(double input1, double input2, double weight1, double weight2);
private:
    double selfWeight;
    double bias;
};

double Hidden::Push_and(double input1, double input2, double weight1, double weight2)
{
        //printf("********(%f)********\n",input2*weight2+bias);
    if(input1*weight1+bias > 0 && input2*weight2+bias > 0){
        return 1.0;
    }
    return 0.0;
}

double Hidden::Push_or(double input1, double input2, double weight1, double weight2)
{
        //printf("********(%f)********\n",input2*weight2+bias);
    if(input1*weight1+bias > 0 || input2*weight2+bias > 0){
        return 1.0;
    }
    return 0.0;
}

double Hidden::Push_s(double input1,double input2,double weight1,double weight2)
{
    double input = -(input1*weight1+input2*weight2+selfWeight+bias);
    double result = 1/(1+std::exp(input));          //SIGMOID function 
        //pc.printf("%f\n",result);
    return result;
}

Hidden::Hidden(double selfweight,double bias)
{
    Hidden::selfWeight = selfweight;
    Hidden::bias = bias;
}

// ThC
Hidden sigmoid17(5,-17.5);                         
Hidden m_Servo14(0,2.75);

// CTr
Hidden sigmoid18(5,8.7);
Hidden sigmoid19(5,12);
Hidden orNeuron20(0,-30.0);
Hidden sigmoid21(5,-16.19);
Hidden sigmoid22(5,-17.96);
Hidden orNeuron23(0,-30.00);
Hidden sigmoid15(16,-8.0);

// FTi
Hidden orNeuron27(0,-12.5);
Hidden sigmoid16(16.0,7.0);

//******************************************************************
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//*******************SERVO MODULE********************************
class Servo
{
public:
    Servo(double bias);
    double ThCServo(double tcAngle, double tcVelocity, double output17, double ang_Weight, double vel_Weight);
    double CTrHeightServo(double tcAngle, double ctAngle, double ftAngle, double tcOffset, double coxaL, double tibiaL, double femurL);
    void FTiServo();
private:
    double bias;
};

double Servo::CTrHeightServo(double tcAngle, double ctAngle, double ftAngle, double tcOffset, double coxaL, double tibiaL, double femurL)
{
    /****
    double ch1 = sin(PI*((3072-ctAngle)*0.088-tcOffset)/180)*femurL;
    double ch2 = sin(PI*((ftAngle-3072+ctAngle)*0.088+tcOffset)/180)*tibiaL;
    double tcangle = abs(tcAngle-2048)*0.088;
    double ch = (ch2-ch1+coxaL*cos(40*PI/180))*cos(tcangle*PI/180)*cos(40*PI/180);
        //printf("***** %f *****\n",ch);
    return -ch + des_Height;
    **/
    // Equation: des_Height = (sin(r-x+f)*Lt-sin(x-f)*Lf+cos(f)*Lc)*cos(tc)*cos(f)
    double r = PI*ftAngle*0.088/180;
    double f = 40*PI/180;
    double tcangle = PI*abs(tcAngle-2048)*0.088/180;
    
    double C = (des_Height/cos(f)/cos(tcangle)-cos(40*PI/180)*coxaL);
    double B = cos(r+f)*tibiaL+cos(f)*femurL;
    double A = sin(r+f)*tibiaL+sin(f)*femurL;
    double X = sqrt((C*C+B*B)/(B*B+A*A));
    double des_ctAngle = 3072- acos(X)*180/PI/0.088;
        //printf("*** %f ***\n",des_ctAngle);
    return des_ctAngle;
}

double Servo::ThCServo(double tcAngle, double tcVelocity, double output17, double ang_Weight, double vel_Weight)
{
	///////////////////////// REVERSE LEGS HERE //////////////////////////////////////////////////////////
	//// Input1 Represents the angle of ThC joint
    double input1 = 4096 - tcAngle;
    double des_p = 0;
    if(tcVelocity > 1023){
        tcVelocity = tcVelocity - 1023;
    }
    else{
        tcVelocity = -tcVelocity;
    }
    double input2 = tcVelocity;
    if(output17 > 0.5){
        des_p  = 2048 - (double)tcPEP;
    }
    
    if(output17 < 0.5){
        des_p  = 2048 - (double)tcAEP;
    }
    double output = input1*ang_Weight + 0.01*input2*vel_Weight + des_p*bias;
    if(output > 0 ){
        return 1;
    }
    else
        return -1;
}

Servo::Servo(double bias)
{
    Servo::bias = bias;
}

Servo servo14(3.7);
Servo servo24(des_Height);
//******************************************************************
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//*******************BISTABLE MODULE********************************
class Bimodule
{
public:
    Bimodule(double bias);
    void tcJointCtrl(double input, double weight, double speed);
    void ctInstepCtrl(double input);
    void ctJointCtrl(double input, double weight, double speed);
    void ftJointCtrl(double input, double weight, double speed);
    void heightCtrl(double input);
private:
    double bias;
};

void Bimodule::ctInstepCtrl(double input)
{
    jointCT.InStepLev(input);
}

void Bimodule::ftJointCtrl(double input, double weight, double speed)
{
    double output = input*weight + bias;
        //printf("///// %f /////\n",output);
    if(output > 0){
        if(output >= 0.9 && output <= 2.0){
            jointFT.Ext(speed);
        }
        
        if(output >= 15.9){
            jointFT.Flx(speed);
        }
    }
}

void Bimodule::ctJointCtrl(double input, double weight, double speed)
{
    double output = input*weight+ bias;
        //printf("***** %f *****\n",output);
    if(output > 0){
        if(output == 8.0){
            jointCT.Lev(speed);
        }
        if(output >= 9.95){
            jointCT.Dep(speed);
        }
    }
}

void Bimodule::tcJointCtrl(double input, double weight, double speed)
{
    double output = input*weight + bias;
        printf("***** %f *****\n",output);
    if(output > 0){
        if(output == 16){
            jointTC.Pro(speed);
        }
        
        if(output == 48){
            jointTC.Ret(speed);
        }
    }
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
Bimodule Levation(-14);
// neuron 11
Bimodule Depression(18);
// neuron 12
Bimodule Flexion(-16);
// neuron 13
Bimodule Extension(7.5);
//******************************************************************
////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////
//*******************CONTROL NETWORK********************************
class Control
{
public:
    //Constructor
    Control(uint8_t id1, uint8_t id2, uint8_t id3);
    //Control Network
    void Control_all(uint16_t speed);
private:
    double output17;
    double output14;
    
	double output18;
    double output19;
    double output21;
    double output15;
    
	double output25;
    double output26;
    double output28;
    double output29;
    double output16;
};

void Control::Control_all(uint16_t speed)
{
    ctrlTimer.reset();
    ctrlTimer.start();
    int handler = 10;
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
            
            //.....................ThC Joint Control............
			// tcAngle and tcVel are neuron1 and neuron2. f_Contact = neuron7
            
			// 7->17 (input1, input2, weight1, weight2)
			output17 = sigmoid17.Push_s(0,f_Contact,0,20);                            // 1 --> Stance, 0 --> Swing
            // 1,2,17->14
			output14 = servo14.ThCServo(tcAngle,tcVel,output17,-4.0,-2.0);
            // 14->8
			Protraction.tcJointCtrl(output14,32,step_speed);
            // 14->9
			Retraction.tcJointCtrl(output14,-32,step_speed);
            
            //.....................CTr Joint Control............
			// 1->18
            output18 = sigmoid18.Push_s((2048-tcAngle)*0.088/90,0,-30.5,0);
            // 5->19
			output19 = sigmoid19.Push_s(0,ftAngle*0.088/180,0,32);
            // 18,19->20
			double output20 = orNeuron20.Push_or(output18,output19,32,32);
                //printf("*****%f,%f,%f,%f,%f******\n", output18, output19, output20, tcAngle*0.088, ftAngle*0.088);
            // 5->21
			output21 = sigmoid21.Push_s(0,ftAngle*0.088/180,0,-32);
			// 20,21->15
			output15 = sigmoid15.Push_s(output20,output21,32,-24);                  // >0.6 --> Levation, <0.6 --> Depression
                //printf("***** %f %f,%f,%f *****\n",output20,output21,output15,ftAngle*0.088);                               
            // HEIGHT CONTROLLER
			if(tcAngle < 2048 && f_Contact == 1.0){                                 // Tibia length:20.5, Femur length: 22.5, Coxa length:5.0
                // 
				double output24 = servo24.CTrHeightServo(tcAngle,ctAngle,ftAngle,40,5.0,20.5,22.5);
                Levation.ctInstepCtrl(output24+50);
                    //printf("***** %f,%f *****\n",output24,ctAngle);
            }
            else{
				// 15->10
                Levation.ctJointCtrl(output15,16,step_speed);
                // 15->11
				Depression.ctJointCtrl(output15,-2.0,step_speed);
            }
            
            //.....................FTi Joint Control............
            //5->25
			output25 = sigmoid25.Push_s(0,ftAngle*0.088/180,0,30);
            //7->26
			output26 = sigmoid26.Push_s(0,f_Contact,0,-20);
            //25,26->27
			double output27 = orNeuron27.Push_or(output25,output26,32,30);
                //printf("***** %f,%f,%f,%f ******\n", output25, output26, output27, ftAngle);
            //5->28
			output28 = sigmoid28.Push_s(0,ftAngle*0.088/180,0,-32);
            // 7->29
			output29 = sigmoid29.Push_s(0,f_Contact,0,20);
            //28,29->30
			double output30 = andNeuron30.Push_and(output28,output29,20,20);
                //printf("***** %f,%f,%f,%f ******\n", output28, output29, output30, ftAngle);
            // 27,30->16
			output16 = sigmoid16.Push_s(output27,output30,-24,32);                //0 --> Extension, 1 --> Flexion
                //printf("***** %f,%f,%f,%f *****\n",output27, output30, output16, ftAngle);
            // 16->12
			Flexion.ftJointCtrl(output16,30,step_speed);
            // 16->13
			Extension.ftJointCtrl(output16,-2.0,step_speed);
            ctrlTimer.reset();
			//????????????????
            handler = handler - 1;
                
        }
        
    }
}

Control::Control(uint8_t id1, uint8_t id2, uint8_t id3)
{
    jointTC.SetId(id1);
    jointCT.SetId(id2);
    jointFT.SetId(id3);
    // FOR SIDEWALKING SET OUTPUT17 TO 0
    //output17 = 0.0;
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
                case Start:                    
                    stepTimer.reset();
                    stepTimer.start();
                    isWalking = true;
                    startStopLED = 1;
                    break;
                case Stop:                    
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