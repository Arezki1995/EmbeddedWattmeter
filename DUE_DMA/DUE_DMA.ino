#undef HID_ENABLED

// ADC->DMA->USB
// Input: Analog in A0 - A10
// Output: Raw stream of uint16_t in range 0-4095 on Native USB Serial/ACM


volatile int bufn,obufn;
uint16_t buf[4][256];   // 4 buffers of 256 readings

//////
int InterruptPin = 2;
int outPin       = 10;
volatile int state = LOW;
 
//////////////////////////////////////////////////////////////////////
void ADC_Handler(){     
    // move DMA pointers to next buffer
    int f=ADC->ADC_ISR;
    if (f&(1<<27)){
        bufn=(bufn+1)&3;
        ADC->ADC_RNPR=(uint32_t)buf[bufn];
        ADC->ADC_RNCR=256;
    } 
}
//////////////////////////////////////////////////////////////////////
void PinRead_ISR(){
 state = !state;
 digitalWrite(outPin, state); 
}

void setupIO(){
   pinMode(outPin, OUTPUT);
   attachInterrupt(digitalPinToInterrupt(InterruptPin), PinRead_ISR, FALLING);
}


//////////////////////////////////////////////////////////////////////
void setup(){
  
  ////
      setupIO();  
  ////
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
  bufn=obufn=1;

  // Transfer Control Register: Receiver Transfer Disable 
  ADC->ADC_PTCR=1;

  // Control Register
  ADC->ADC_CR=2;
}

//////////////////////////////////////////////////////////////////////
void loop(){
  // wait for buffer to be full
    while(obufn==bufn);
    
  // send it - 512 bytes = 256 uint16_t
  SerialUSB.write((uint8_t *)buf[obufn],512); 
  
  // Increment buffer index
  obufn=(obufn+1)&3;    
}