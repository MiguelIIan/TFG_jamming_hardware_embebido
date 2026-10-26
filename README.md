# TFG_jamming_hardware_embebido

<div align=justify>

En este repositorio se encuentran los elementos digitales necesarios para la construcción de un eslabón robótico de rigidez variable basado en "jamming" con todo el hardware necesario embebido. Aquí vienen almacenados tanto los modelos 3D de todas las piezas del eslabón como los códigos con los que hacerlo funcionar y los datos obtenidos experimentalmente.

## Estructura del proyecto

* [Modelos 3D/](https://github.com/MiguelIIan/TFG_jamming_hardware_embebido/tree/main/Modelos%203D): Contiene las piezas modeladas en 3D en la aplicación **Fusion 360**, almacenados en 3 conjuntos diferentes.
* [NodoEsp32/](https://github.com/MiguelIIan/TFG_jamming_hardware_embebido/tree/main/NodoEsp32): Aquí se encuentran los códigos Arduino con los que se puede controlar el eslabón a través de Micro-ROS.
* [Pruebas/](https://github.com/MiguelIIan/TFG_jamming_hardware_embebido/tree/main/Pruebas): Vienen almacenados los datos de los experimentos realizados a lo largo del desarrollo de este proyecto.
* [robotica_blanda](https://github.com/MiguelIIan/TFG_jamming_hardware_embebido/tree/main/robotica_blanda): Paquete ROS2 con todos los códigos para conectar el eslabón de rigidez variable a la interfaz gráfica y así poder controlarlo con facilidad.

## Requisitos
* **ROS2 Jazzy Jalisco**. Probado en Ubuntu 24.04.
* **Arduino IDE** con soporte para ESP32 y la librería [Micro-ROS Jazzy](https://github.com/micro-ROS/micro_ros_setup/tree/jazzy).
* Para las representaciones: Matlab.
* Librerías python para el funcionamiento de la interfaz gráfica.
   * **PyQt5**: Librería para la construcción de la UI. Para instalarlo: ```sudo apt install python3-pyqt5```.
   * **rclpy** y **std_msgs.msg**: Librería para la comunicación entre la interfaz y el entorno ROS2. Para instalarlo: ```sudo apt install ros-jazzy-rclpy ros-jazzy-std-msgs```

## Construcción del entorno
* Compilar el archivo .ino escogido y programar el ESP32.
* Crear el workspace ROS2 en el que se quiera incluir este paquete [robotica_blanda](https://github.com/MiguelIIan/TFG_jamming_hardware_embebido/tree/main/robotica_blanda) y compilarlo con ```colcon build```.

## Ejecución del proceso









</div>
