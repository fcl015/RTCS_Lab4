%m-file to adquire ERIKA-FLEX FULL data from the serial port using RS232 

clear all
clc
tic 

%------------------------------------------------------------------------
% Configure communication parameter
%------------------------------------------------------------------------
number_of_samples=600;                  %set the number of adquisitions
offset_samples=0;
                                        %configuring serial port
s=serial('COM3');                      %creates a matlab object from the serial port 'COM1' 
s.BaudRate=115200;                      %115200;%230400;%460800;%baudrate=115200bps
s.Parity='none';                        %no parity
s.DataBits=8;                           %data sended in 8bits format
s.StopBits=1;                           %1 bit to stop
s.FlowControl='none';                   %no flowcontrol
s.Terminator='LF';                      %LineFeed character as terminator
s.Timeout=10;                           %maximum time in seconds since the data is readed
s.InputBufferSize=100000;               %increment this value when data is corrupted
fopen(s)                                %open serial port object

disp('Waiting for initial data... ')
for n=1:offset_samples
    data=fread(s,6,'char');
    if ( data(1) ~= 1 )
        disp('Communication problems....please re-try');
        fclose(s)             
        delete(instrfind)     
        return
    end
end
disp('Capturing data... ')

%------------------------------------------------------------------------
% Capture data from Explorer 16
%------------------------------------------------------------------------
task1=[];
task2=[];
task3=[];
my_time=[];
for n = 1:number_of_samples             %get data and plot 
    data=fread(s,6,'char');
    my_time(n,1)=(bitshift(data(2),8) + data(3))/10;
    
    task1(n,1)=data(4);
    task2(n,1)=data(5);
    task3(n,1)=data(6);
       
    subplot(3,1,1,'replace'); plot(my_time,task1,'b.-');
    axis([my_time(1) my_time(1)+60 -0.1 2.1]);grid on;
    ylabel('TASK 1');
    subplot(3,1,2,'replace'); plot(my_time,task2,'b.-');
    axis([my_time(1) my_time(1)+60 -0.1 2.1]);grid on;
    ylabel('TASK 2');
    subplot(3,1,3,'replace'); plot(my_time,task3,'b.-');
    axis([my_time(1) my_time(1)+60 -0.1 2.1]);grid on;
    ylabel('TASK 3');xlabel('time (seconds)');
    drawnow; 
end


%------------------------------------------------------------------------
% Close serial port communication
%------------------------------------------------------------------------
fclose(s)                               %close serial port object
delete(instrfind)                       %clean all open ports
