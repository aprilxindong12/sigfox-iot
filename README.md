# sigfox-iot

## [Building an IoT platform for remote monitoring of field applications using Sigfox LPWAN](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/poster_A0.pdf)

**Field applications at remote locations** are often diffcult to monitor and maintain. Frequent site visits incur high labour costs and unawareness of the accurate condition of the sites can lead to incomprehensive managing decisions. To address these problems, an **IoT** system can be developed and installed in the field to achieve **remote monitoring** of the field applications.

The project aims to design, build and test an **IoT** platform that can be used to monitor two selected field applications at remote locations. The three **IoT** applications that have been studied are **water level and height monitoring**, **monitoring volumetric percentage of moisture in the soil of the garden** and **monitoring soil moisture level in landfill remediation fields**. **Sigfox** is the **LPWAN** used in these applications which is able to provide a coverage of ***3-10km*** in urban places and ***30-50km*** in rural areas. For each application, components of the **IoT** system have been selected and connected, software programs have been written for the system and a user interface has been set up on the Internet. There are two parts of the **IoT** system; one is the controller, which is called the **IoT** solution, this is the same for all applications, the other is the sensor specific for each application.

On successful completion of the project, an end-to-end **IoT** application using the **Sigfox** network has been implemented; and the total cost of the developed **IoT** solution is around ***$70***, which is ***14%*** of the price of a similar commercial **Sigfox** **IoT** product. Two field units to monitor soil moisture in the garden and a landfill remediation field respectively have been constructed and deployed. The next step is to a design and manufacture a PCB with an **Arduino**-compatible microchip, a **Sigfox** module, an effcient voltage regular and other modules that together make it a low-power board.

## Three IoT Applications

### [Water level and height monitoring](http://xkit-2bebc3.mybluemix.net/ui)

**Charts for real-time data on the IBM Watson IoT Platform**

![water_dashboard](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/dashboard/water_dashboard.PNG)

**Dashboard for real-time data set up using Node-RED flows**

![water_dashboard2](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/dashboard/water_dashboard2.PNG)

### [Monitoring volumetric percentage of moisture in the soil of the garden](http://xkit-3e5ac8.mybluemix.net/ui)

**Layout of the IoT system**

![layout](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/garden_layout.png)

**Layout of the IoT system with power management features**

![layout_power](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/garden_power_layout.png)

**Instances of the dashboard when real-time data present**

![garden_dashboard](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/dashboard/garden_dashboard.PNG)

![garden_dashboard2](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/dashboard/garden_dashboard2.PNG)

![garden_dashboard3](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/dashboard/garden_dashboard3.PNG)

### [Monitoring soil moisture level in landfill remediation fields](http://xkit-2c5ca8.mybluemix.net/ui)

**Layout of the IoT field unit**

![layout2](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/soil_layout.png)

**The real-time data tab of the dashboard in use**

![soil_dashboard](https://github.com/aprilxindong12/sigfox-iot/blob/master/docs/dashboard/soil_dashboard.PNG)

### Libraries

* [AVR libraries](https://www.nongnu.org/avr-libc/user-manual/modules.html)

* [Thinxtra/Xkit-Sample](https://github.com/Thinxtra/Xkit-Sample)

* [SDISerial](https://github.com/joranbeasley/SDISerial)
