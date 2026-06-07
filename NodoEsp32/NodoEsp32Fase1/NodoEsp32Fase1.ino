#include <micro_ros_arduino.h>

#include <stdio.h>
#include <rcl/rcl.h>
#include <rcl/error_handling.h>
#include <rclc/rclc.h>
#include <rclc/executor.h>

#include <std_msgs/msg/int32.h>
#include <std_msgs/msg/float32.h>

//--------------Recordatorios--------------------------------
// - Para ejecutar el agente de micro_ros --> "ros2 run micro_ros_agent micro_ros_agent serial --dev /dev/ttyUSB0"
// - Para medir la presion los sensores dan la información en intensidad, pero con una resistencia podemos obtener la tensión y medirla con el "analogRead(PIN DE LA PLACA)", 
// que va de 0 a 4095 de 0 V a 3.3 V. El sensor va de -1 bar a 1 bar dando de 4 mA a 20 mA, por lo que un buen valor de resistencia para poder hacer la medida es 250 Ohmios.
//-----------------------------------------------------------

// Definimos las variables que contendrán a nuestro publicadores, subscriptores, mensajes y las partes esenciales para que funcione el entorno ROS2
rcl_publisher_t publishercamara;
rcl_publisher_t publisherblanda;
rcl_publisher_t debugger;
rcl_subscription_t subscriptioncontrol;

std_msgs__msg__Float32 msgcamaravacio;
std_msgs__msg__Float32 msgparteblanda;
std_msgs__msg__Int32 msgcontrol;
std_msgs__msg__Float32 msg;

rclc_executor_t executor;
rclc_support_t support;
rcl_allocator_t allocator;
rcl_node_t node;
rcl_timer_t timer;

//Definicion de pines
#define SENSORBLANDO 12
#define SENSORVACIO 13
#define BOMBA 25
#define VALVULA_BLANDA 26  // En los cables de la placa con los relays, el cable blanco es el destinado al relay de la izquierda mientras que el amarillo es el de la derecha
#define VALVULA_CAMARA 27

// Definiciones para el funcionamiento de ROS2
#define RCCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){error_loop();}}
#define RCSOFTCHECK(fn) { rcl_ret_t temp_rc = fn; if((temp_rc != RCL_RET_OK)){}}

// Variables
float presioncamaravacio = 0.0;
float presionparteblanda = 0.0;
float valorcontrol = 0.0;
unsigned long valor0 = 0;

float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow; 
}

void error_loop(){
  while(1){
    digitalWrite(17, !digitalRead(17));
    delay(100);
  }
}

// Funciones callbacks para los topicos a los que el ESP32 está suscrito
void control_callback(const void * msgin)
{  
  const std_msgs__msg__Int32 * mensaje = (const std_msgs__msg__Int32 *)msgin;
  /*msg.data = mensaje->data;
  RCSOFTCHECK(rcl_publish(&debugger, &msg, NULL));*/

  valorcontrol=mapFloat(mensaje->data,0.0,100.0,0.0,-0.45);
  msg.data = valorcontrol;
  RCSOFTCHECK(rcl_publish(&debugger, &msg, NULL));
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL) {
    msgcamaravacio.data = presioncamaravacio;
    msgparteblanda.data = presionparteblanda;
    // Publicamos en los topicos el mensaje que queramos
    RCSOFTCHECK(rcl_publish(&publishercamara, &msgcamaravacio, NULL));
    RCSOFTCHECK(rcl_publish(&publisherblanda, &msgparteblanda, NULL));
    //msg.data++;
  }
}

void setup() {
  set_microros_transports();
  
  pinMode(VALVULA_BLANDA,OUTPUT); 
  pinMode(VALVULA_CAMARA,OUTPUT); 
  pinMode(BOMBA,OUTPUT);
  
  delay(2000);

  allocator = rcl_get_default_allocator();

  //create init_options
  RCCHECK(rclc_support_init(&support, 0, NULL, &allocator));

  // create node
  RCCHECK(rclc_node_init_default(&node, "esp32_node", "", &support));
  
  // ----------Create subscriber------------
  RCCHECK(rclc_subscription_init_default(
    &subscriptioncontrol,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "ControlTeclado"));
  
  // --------Create publisher-------------
  RCCHECK(rclc_publisher_init_default(
    &publishercamara,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "CamaraVacio"));
    
  RCCHECK(rclc_publisher_init_default(
    &publisherblanda,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "ParteBlanda"));

  RCCHECK(rclc_publisher_init_default(
    &debugger,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Float32),
    "Debug"));

  // create timer,
  const unsigned int timer_timeout = 100;
  RCCHECK(rclc_timer_init_default(
    &timer,
    &support,
    RCL_MS_TO_NS(timer_timeout),
    timer_callback));

  // Creamos el ejecutador y definimos las subscripciones que va a tener el nodo
  RCCHECK(rclc_executor_init(&executor, &support.context, 2, &allocator)); // Necesitamos decirle al ejecutor cuantas entidades(callbacks) va a tener que ejecutar (1 timer y 1 subscriber)
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriptioncontrol, &msgcontrol, &control_callback, ON_NEW_DATA));

  msgcamaravacio.data = 0;
  msgparteblanda.data = 0;

  digitalWrite(VALVULA_BLANDA,HIGH);
  digitalWrite(VALVULA_CAMARA,LOW);
  delay(2000);
  valor0 = analogRead(SENSORBLANDO)+10;
  digitalWrite(VALVULA_BLANDA,LOW);
  digitalWrite(VALVULA_CAMARA,HIGH);
  delay(2000);
  digitalWrite(VALVULA_CAMARA,LOW);
}

unsigned long ahora = 0;
unsigned long temporizadormsg = 0;
unsigned long temporizadorsensores = 0;
unsigned long contador = 0;
bool valvulablandaactiva = false;
bool valvulacamaraactiva = false;
bool bombaactiva = false;

void loop() {
  ahora = millis();

  if(ahora-temporizadormsg>=100){
    // Ejecutamos un poco el nodo y liberamos el microcontrolador
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
    temporizadormsg = ahora;
  }
  
  if(ahora-temporizadorsensores >= 10){
    presioncamaravacio = mapFloat(analogRead(SENSORVACIO),650,valor0,-1.0,0.0); // Con una resistencia de 120 Ohms nos va a dar la presión en bares entre -1 y 1.   
    presionparteblanda = mapFloat(analogRead(SENSORBLANDO),650,valor0,-1.0,0.0); // La mejor resistencia sería de 240 Ohms y mediríamos solo la presión entre -1 y 0.

    
    if(presioncamaravacio >= -0.52 && bombaactiva == false){
      bombaactiva = true;
    }
    else if(presioncamaravacio <= -0.57 && bombaactiva == true){
      bombaactiva = false;
    }
    int estadobomba = (bombaactiva) ? HIGH:LOW;
    digitalWrite(BOMBA,estadobomba);
    
    if(presioncamaravacio >= -0.45){
      /*if(presionparteblanda <= valorcontrol-0.2 && valorcontrol == 0.0){
        digitalWrite(VALVULA_1,HIGH);
        digitalWrite(VALVULA_2,LOW);
      } else {*/
        valvulablandaactiva = false;
        valvulacamaraactiva = false;
      //}
    }
    else{
      if(valorcontrol <= -0.2){
        valvulablandaactiva = false;
        if(presionparteblanda >= valorcontrol+0.3 && valvulacamaraactiva == false){
          valvulacamaraactiva = true;
        } else if(presionparteblanda <= valorcontrol-0.1 && valvulacamaraactiva == true){
          valvulacamaraactiva = false;
        } 
        if(valvulacamaraactiva){
          digitalWrite(VALVULA_BLANDA,LOW);
          digitalWrite(VALVULA_CAMARA,HIGH);
        } else {
          digitalWrite(VALVULA_BLANDA,LOW);
          digitalWrite(VALVULA_CAMARA,LOW);
        }
        
      } else {
        
        if(valvulablandaactiva == false && presionparteblanda <= -0.2){
          valvulablandaactiva = true;
          contador = ahora;
        }
      } 
    }
    if(valvulablandaactiva == true){
      digitalWrite(VALVULA_BLANDA,HIGH);
      digitalWrite(VALVULA_CAMARA,LOW);
      
    } else {
      digitalWrite(VALVULA_BLANDA,LOW);
    }    
    temporizadorsensores = ahora;
  }
  if(ahora - contador >= 2500 && valvulablandaactiva == true){
    valvulablandaactiva = false;
    digitalWrite(VALVULA_BLANDA,LOW);
  }
}
