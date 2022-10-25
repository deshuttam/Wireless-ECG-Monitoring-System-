# Wireless ECG Monitoring System With Remote Data Logging using PSoC

By Uttam U. Deshpande et. al.,

Programmable System on Chip (PSoC) processor performs rapid, complex signal processing and consumes low power. These PSoC’s capabilities extend its use in designing intelligent wireless sensor node. ECG detection and processing system detects R peaks at regular intervals, calculate heart rate and classify it. High accuracy, low error rate and good noise immunity is achieved by simple thresholding technique. For longer sensor node life, power consumption plays a very essential role. Instead of logging data continuously to the base station, intelligent sensor node enables transmitter only when critical heart rate is observed. Further reduction in power consumption is achieved by using low power CY3271 sensor node itself. Power analysis shows reduction in power due to reduced transmission rate. This results in low traffic over the network. 
Refer the following papers to understand the implementation details 
* [Wireless ECG monitoring system with remote data logging using PSoC and CyFi](https://www.ijareeie.com/upload/june/87_Wireless.pdf)
* [IoT based Real Time ECG Monitoring System using Cypress WICED](https://www.ijareeie.com/upload/2017/february/35_IoT.pdf)

## Introduction 
We have developed a novel approach of interfacing a highly versatile Programmable System On Chip (PSoC) with wireless sensor node CY 3271 sensor node, both from Cypress semiconductors. PSoC enables greater integration, good power efficiency and higher accuracy at lower price. A PSoC mixed signal array is a low power programmable Systems on Chip (SoC). This allows configuring, programming of analog and digital components that are typically used in embedded systems. It also has a built in microcontroller which integrates and controls all of the programmed components. PSoC extends its computational capabilities by using mixed signal array, complex computations such as filtering, compression or suppression can be implemented at individual nodes. This affects in reduction of the data throughput over the network. Hence results in reduced transmission time and increased battery life of sensor node.
![image](https://user-images.githubusercontent.com/107185323/197836454-ab1dbd4b-c71a-4778-8456-5e1aad25d6a3.png)

## ECG Signal Processing and Detection system
ECG signal from simulator is fed as input to this system. This system mainly contains Analog to Digital Converter (ADC), filters, peak detector and LCD.
![image](https://user-images.githubusercontent.com/107185323/197836714-d9b4a01e-7bb9-49cf-a181-09a83a1b3f6f.png)

## ECG Signal Transmitter
Cypress CyFi CY3271 sensor node consists of an application PSoC and RF 7936 radio module. This application MCU is used to implement analog and digital blocks. It controls sensors, peripherals and RF transceiver. SPI, Buffer and CYFISNP modules are implemented in this application PSoC.
![image](https://user-images.githubusercontent.com/107185323/197836971-03f2f272-3692-4870-807d-29ebf2f10a5c.png)

## ECG Signal Receiver (Hub)
RF Receiver consists of RF transceiver, I2C application interface, memory management, I2C slave interface and data buffer. This receiver module consists of two CY8C24894 PSoC’s. Slave PSoC controls RF transceiver for reception of data. Master PSoC provides USB to I2C bridge functionality to communicate with PC or laptop.
![image](https://user-images.githubusercontent.com/107185323/197837206-29132683-e720-49a2-bb05-3dc29a6d55aa.png)

PSoC internal Schematic
![image](https://user-images.githubusercontent.com/107185323/197837435-7ee9efa7-fa8f-4287-88bc-201a0b279ead.png)

## Heart Rate Output
Bradycardia: If heart rate falls below 60 BPM, the condition will be called as “Bradycardia”.
![image](https://user-images.githubusercontent.com/107185323/197837821-fe62a237-489e-4da2-97ff-81b051f63808.png)

Normal Heart Rate: Normal heart rate ranges from 60 to 90 BPM. 
![image](https://user-images.githubusercontent.com/107185323/197837986-061a94ff-339d-4288-b1b6-03a89dd5ad17.png)

Tachycardia: It is the stage heart beats at the rate above 90 BPM. 
![image](https://user-images.githubusercontent.com/107185323/197838132-d0159a5a-2f02-41a7-acaf-8a691e4df5ea.png)

### The repository includes:
* `pulsometer_final` source code for ECG Detection and Processing operation.
* `RF_ULP_TEMP_try` Source code for  RF Transmitter and Receiver operation implementation. 
* Complete implementation report
* Working Videos 

Citing
@ARTICLE{ ijareeie.com/upload/june/87_Wireless.pdf, AUTHOR={Deshpande, Uttam U. et al.,}, TITLE={Wireless ECG monitoring system with remote data logging using PSoC and CyFi}, JOURNAL={International Journal of Advanced Research in Electrical, Electronics and Instrumentation Engineering}, VOLUME={2}, YEAR={2013}, URL={ https://www.ijareeie.com/upload/june/87_Wireless.pdf }, DOI={ ijareeie.com/upload/june/87_Wireless.pdf }, }

@ARTICLE{ ijareeie.com/upload/2017/february/35_IoT.pdf, AUTHOR={Deshpande, Uttam U. et al.,}, TITLE={IoT based Real Time ECG Monitoring System using Cypress WICED}, JOURNAL={International Journal of Advanced Research in Electrical, Electronics and Instrumentation Engineering}, VOLUME={6}, YEAR={2017}, URL={ https://www.ijareeie.com/upload/2017/february/35_IoT.pdf }, DOI={ ijareeie.com/upload/2017/february/35_IoT.pdf }, }


Requirements: Software
* PSoC Designer 5.0
* PSoC Programmer 3.0
* Sense and Control dashboard 2.0 

## Usage
Run `main.c` code present `pulsometer_final` folder for ECG Detection and Processing operation.
Run `main 26-5-13.c` code present `RF_ULP_TEMP_try` folder for RF Transmitter and Receiver operation implementation. 
Observe the working of the project [Videos](https://drive.google.com/drive/folders/1LUzLhPaz7V2EtEPGK05C7IlRYK8ptlez?usp=sharing) 

