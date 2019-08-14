    #include "header.h"

    #undef HID_ENABLED

    // ADC->DMA->USB
    // Input: Analog in A0 - A10
    // Output: Raw stream of uint16_t in range 0-4095 on Native USB Serial/ACM

    int GLOBAL_STATE;

    volatile int bufferIndex,outputBuffer;
    uint16_t buf[4][256];   // 4 buffers of 256 readings

    //////
    unsigned int InterruptPin = 52;

    //pin 13 is the Enable
    static unsigned int MuxSelect[]={11,12,13};

    ///////////////// !!!!!!!!!!!!! ////////////////////////
    // En prenant compte de l'erreur de soudure du prototype
    static unsigned int CM[]={50, 51, 48, 49, 46, 47};

    // sur les schémas electriques originaux
    //static unsigned int CM[]={51, 50, 49, 48, 47, 46};

    // si une carte a été refaite sur les schémas originaux 
    // il faut utiliser la deuxième déclaration

    //////////////////////////////////////////////////////////////////////
    void ADC_Handler(){     
        // move DMA pointers to next buffer
        int f=ADC->ADC_ISR;
        if (f&(1<<27)){
            //outputBuffer=(bufferIndex&7);
            bufferIndex=(bufferIndex+1)&3;
            ADC->ADC_RNPR=(uint32_t)buf[bufferIndex];
            ADC->ADC_RNCR=256;
        } 
    }
    //////////////////////////////////////////////////////////////////////
    void PinRead_ISR(){
    
    // Get the state configuration        
    for(int i=0; i<6; i++){
        int val = digitalRead(CM[i]);
        if(val==0){
            GLOBAL_STATE = GLOBAL_STATE & (~(0x0001<<i));
        }else{
            GLOBAL_STATE = GLOBAL_STATE | ( (0x0001<<i));
        }
    }
    // set ADC Mode
    int ADC_MODE = GLOBAL_STATE & ADC_MODE_MSK;
    
    switch(ADC_MODE) {
        case ADC_MODE_A0   : ADC->ADC_CHER=ADC_A0_CH;  ADC->ADC_CHDR=(~ADC_A0_CH);  break; 
        case ADC_MODE_A1   : ADC->ADC_CHER=ADC_A1_CH;  ADC->ADC_CHDR=(~ADC_A1_CH);  break; 
        case ADC_MODE_A2   : ADC->ADC_CHER=ADC_A2_CH;  ADC->ADC_CHDR=(~ADC_A2_CH);  break; 
        case ADC_MODE_A3   : ADC->ADC_CHER=ADC_A3_CH;  ADC->ADC_CHDR=(~ADC_A3_CH);  break; 
        case ADC_MODE_A4   : ADC->ADC_CHER=ADC_A4_CH;  ADC->ADC_CHDR=(~ADC_A4_CH);  break; 
        case ADC_MODE_A5   : ADC->ADC_CHER=ADC_A5_CH;  ADC->ADC_CHDR=(~ADC_A5_CH);  break; 
        case ADC_MODE_A6   : ADC->ADC_CHER=ADC_A6_CH;  ADC->ADC_CHDR=(~ADC_A6_CH);  break;  
        case ADC_MODE_A7   : ADC->ADC_CHER=ADC_A7_CH;  ADC->ADC_CHDR=(~ADC_A7_CH);  break;  
        case ADC_MODE_A8   : ADC->ADC_CHER=ADC_A8_CH;  ADC->ADC_CHDR=(~ADC_A8_CH);  break; 
        case ADC_MODE_A9   : ADC->ADC_CHER=ADC_A9_CH;  ADC->ADC_CHDR=(~ADC_A9_CH);  break; 
        case ADC_MODE_A10  : ADC->ADC_CHER=ADC_A10_CH; ADC->ADC_CHDR=(~ADC_A10_CH); break;
        case ADC_MODE_A11  : ADC->ADC_CHER=ADC_A11_CH; ADC->ADC_CHDR=(~ADC_A11_CH); break;
        
        case ADC_MODE_666K : _CLEAR_PRESCALER(ADC->ADC_MR);   ADC->ADC_MR|=(0x00<<8);   break;   
        case ADC_MODE_280K : _CLEAR_PRESCALER(ADC->ADC_MR);   ADC->ADC_MR|=(0x04<<8);   break;
        case ADC_MODE_125K : _CLEAR_PRESCALER(ADC->ADC_MR);   ADC->ADC_MR|=(0x09<<8);   break;
        case ADC_MODE_60K  : _CLEAR_PRESCALER(ADC->ADC_MR);   ADC->ADC_MR|=(0x1F<<8);   break;
            
        default : 
        ADC->ADC_CHER=ADC_A0_CH; _CLEAR_PRESCALER(ADC->ADC_MR);   ADC->ADC_MR|=(0x00<<8); break;
    }

    int MUX_SELECT = ((GLOBAL_STATE & 0xF0)>>4); 
    switch(MUX_SELECT){
        case MUX_I1_I3_I5:
            digitalWrite(MuxSelect[2],HIGH); digitalWrite(MuxSelect[1],LOW); digitalWrite(MuxSelect[0],LOW);    
        break;
        
        case MUX_I2_I4_I7:
            digitalWrite(MuxSelect[2],HIGH); digitalWrite(MuxSelect[1],LOW); digitalWrite(MuxSelect[0],HIGH);    
        break;
        
        case MUX_I6_I8_I10:
            digitalWrite(MuxSelect[2],HIGH); digitalWrite(MuxSelect[1],HIGH); digitalWrite(MuxSelect[0],LOW);    
        break;
        
        case MUX_I0_I9_I11:
            digitalWrite(MuxSelect[2],HIGH); digitalWrite(MuxSelect[1],HIGH); digitalWrite(MuxSelect[0],HIGH);    
        break;
        
        default: break;
    }


    }
    //////////////////////////////////////////////////////////////////////
    void setupIO(){
    // input command bus
    for(int i=0; i<6;i++){
        pinMode(CM[i], INPUT);
    }

    // output Mux select bus
    for(int i=0; i<3;i++){
        pinMode(MuxSelect[i], OUTPUT);
    }
    
    // interruptPin
    pinMode(InterruptPin, INPUT);
    attachInterrupt( digitalPinToInterrupt(InterruptPin), PinRead_ISR, FALLING);  
    }
    //////////////////////////////////////////////////////////////////////
    void setup(){
    
    setupIO();
    
    SerialUSB.begin(0);
    while(!SerialUSB);

    // Ask power manager controller to activate ADC module
    pmc_enable_periph_clk(ID_ADC);

    // Initialize ADC clock to Max and enable fast wakeups
    adc_init(ADC, SystemCoreClock, ADC_FREQ_MAX, ADC_STARTUP_FAST);
    
    // Free running
    ADC->ADC_MR |=0x80; 

    // Select ADC Channel 7 -> A0 by default
    ADC->ADC_CHER=0x80; 

    // Enable ADC interrupt
    NVIC_EnableIRQ(ADC_IRQn);

    // ENDRX: End of Receive Buffer Interrupt Disable
    ADC->ADC_IDR=~(1<<27);

    // ENDRX: End of Receive Buffer Interrupt Enable
    ADC->ADC_IER=1<<27;
    
    // DMA buffer: pointer to first buffer
    ADC->ADC_RPR=(uint32_t)buf[0];   

    // Size of the DMA buffer
    ADC->ADC_RCR=256;

    // Pointer to next DMA buffer
    ADC->ADC_RNPR=(uint32_t)buf[1]; 

    // Size of the next DMA buffer
    ADC->ADC_RNCR=256;

    // initialize output buffer index
    bufferIndex=outputBuffer=1;

    // Transfer Control Register: Receiver Transfer Disable 
    ADC->ADC_PTCR=1;

    // Control Register
    ADC->ADC_CR=2;
    }

    //////////////////////////////////////////////////////////////////////
    void loop(){
        // wait for buffer to be full
        while(outputBuffer==bufferIndex);
            
        // send it - 512 bytes = 256 uint16_t

        SerialUSB.write((uint8_t *)buf[outputBuffer],512);
        
        // Increment buffer index
        outputBuffer=bufferIndex;
    }