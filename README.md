APEIRON is a framework encompassing the general architecture
of a distributed heterogeneous processing platform and the corresponding software stack, from the low level device drivers up to the high level programming
model. Developers can define scalable applications that can be deployed on a multi-FPGA system coding at high level: the APEIRON communication IPs allow low-latency 
communication between processing tasks deployed on FPGAs, even if hosted on different computing nodes. Thanks to the use of High Level Synthesis tools, tasks are 
described in high level language (C/C++) while communication is expressed through a lightweight API.
The original aim of the APEIRON project was to develop a flexible framework that could be adopted in the design and implementation of "intelligent" low level 
trigger systems and of data reduction stages in trigger-less or streaming readout experimental setups, but it can be used for any kind of real-time distributed stream processing
on multi-FPGA systems.
