# Visualización del funcionamiento del sistema

### Procedimiento

<div align="justify">
Para poder visualizar en gráficas el comportamiento del sistema, se hará uso de las rosbags implemantadas en ROS2 en conjunto con la herramienta Plotjuggler. Para ello, se seguirán ciertos pasos.

<b>

1. Lanzar todos los nodos del sistema para ponerlo en funcionamiento.

2. Abrir un terminal aparte para ejecutar el comando con el que grabaremos todos los datos de los tópicos que queremos visualizar. El comando es ```ros2 bag record --all -o <Nombre de la rosbag>```. Este comando graba la información de todos los tópicos activos y lo guarda en un archivo con el nombre indicado.

3. Una vez se ha grabado todo con la "rosbag", abrimos Plotjuggler. Aquí, se activará la opción de escucha. En un terminal, ejecutaremos el comando ```ros2 bag play <Dirección del archivo rosbag>```.

4. En Plotjuggler observaremos como empiezan a aparecer los datos. Deberemos arrastrar los datos que queramos ver al hueco de las gráficas.

</b>
De esta manera, obtendremos unas gráficas que representan los valores recibidos y dados por el sistema con las que podremos evaluar su  comportamiento y correcto funcionamiento.
</div>