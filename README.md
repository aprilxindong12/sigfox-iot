# sigfox-iot

## Building an IoT platform for remote monitoring of field applications using Sigfox LPWAN

**Field applications at remote locations** are often diffcult to monitor and maintain. Frequent site visits incur high labour costs and unawareness of the accurate condition of the sites can lead to incomprehensive managing decisions. To address these problems, an **IoT** system can be developed and installed in the field to achieve **remote monitoring** of the field applications.

The project aims to design, build and test an **IoT** platform that can be used to monitor two selected field applications at remote locations. The three **IoT** applications that have been studied are **water level and height monitoring**, **monitoring volumetric percentage of moisture in the soil of the garden** and **monitoring soil moisture level in landfill remediation fields**. **Sigfox** is the **LPWAN** used in these applications which is able to provide a coverage of ***3-10km*** in urban places and ***30-50km*** in rural areas. For each application, components of the **IoT** system have been selected and connected, software programs have been written for the system and a user interface has been set up on the Internet. There are two parts of the **IoT** system; one is the controller, which is called the **IoT** solution, this is the same for all applications, the other is the sensor specific for each application.

On successful completion of the project, an end-to-end **IoT** application using the **Sigfox** network has been implemented; and the total cost of the developed **IoT** solution is around ***$70***, which is ***14%*** of the price of a similar commercial **Sigfox** **IoT** product. Two field units to monitor soil moisture in the garden and a landfill remediation field respectively have been constructed and deployed. The next step is to a design and manufacture a PCB with an **Arduino**-compatible microchip, a **Sigfox** module, an effcient voltage regular and other modules that together make it a low-power board.

# Used libraries

* [SDISerial](https://github.com/joranbeasley/SDISerial)

* [Thinxtra/Xkit-Sample](https://github.com/Thinxtra/Xkit-Sample)
