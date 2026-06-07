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
rcl_subscription_t subscriptionapagar;

std_msgs__msg__Float32 msgcamaravacio;
std_msgs__msg__Float32 msgparteblanda;
std_msgs__msg__Int32 msgcontrol;
std_msgs__msg__Int32 msgapagar;
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
float Presion_vacio = 0.0;
float Presion_blanda = 0.0;
float valorControl = 0.0;
unsigned long valor0 = 0;
bool apagado = false;

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

  valorControl=mapFloat(mensaje->data,0.0,100.0,0.0,-0.45);
  msg.data = valorControl;
  RCSOFTCHECK(rcl_publish(&debugger, &msg, NULL));
}

void apagar_callback(const void * msgin)
{  
  const std_msgs__msg__Int32 * mensaje = (const std_msgs__msg__Int32 *)msgin;

  digitalWrite(BOMBA,LOW);
  digitalWrite(VALVULA_BLANDA,HIGH);
  digitalWrite(VALVULA_CAMARA,HIGH);
  delay(10000);
  digitalWrite(VALVULA_BLANDA,LOW);
  digitalWrite(VALVULA_CAMARA,LOW);

  apagado = apagado ? false : true;

  if(!apagado) calibrar_sensores();
}

void timer_callback(rcl_timer_t * timer, int64_t last_call_time)
{  
  RCLC_UNUSED(last_call_time);
  if (timer != NULL && !apagado) {
    msgcamaravacio.data = Presion_vacio;
    msgparteblanda.data = Presion_blanda;
    // Publicamos en los topicos el mensaje que queramos
    RCSOFTCHECK(rcl_publish(&publishercamara, &msgcamaravacio, NULL));
    RCSOFTCHECK(rcl_publish(&publisherblanda, &msgparteblanda, NULL));
    //msg.data++;
  }
}

void calibrar_sensores(){
  digitalWrite(VALVULA_BLANDA,HIGH);
  digitalWrite(VALVULA_CAMARA,HIGH);
  delay(10000);
  valor0 = analogRead(SENSORVACIO)+10;
  delay(1000);
  digitalWrite(VALVULA_BLANDA,LOW);
  digitalWrite(VALVULA_CAMARA,LOW);
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

  RCCHECK(rclc_subscription_init_default(
    &subscriptionapagar,
    &node,
    ROSIDL_GET_MSG_TYPE_SUPPORT(std_msgs, msg, Int32),
    "Apagar"));
  
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
  RCCHECK(rclc_executor_init(&executor, &support.context, 3, &allocator)); // Necesitamos decirle al ejecutor cuantas entidades(callbacks) va a tener que ejecutar (1 timer y 2 subscriber)
  RCCHECK(rclc_executor_add_timer(&executor, &timer));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriptioncontrol, &msgcontrol, &control_callback, ON_NEW_DATA));
  RCCHECK(rclc_executor_add_subscription(&executor, &subscriptionapagar, &msgapagar, &apagar_callback, ON_NEW_DATA));

  msgcamaravacio.data = 0;
  msgparteblanda.data = 0;

  calibrar_sensores();
}

unsigned long ahora = 0;
unsigned long temporizadormsg = 0;
unsigned long temporizadorsensores = 0;
unsigned long contador = 0;
bool valvula_blanda_activa = false;
bool valvula_camara_activa = false;
bool bombaactiva = false;

void loop() {
  ahora = millis();

  if(ahora-temporizadormsg>=100){
    // Ejecutamos un poco el nodo y liberamos el microcontrolador
    RCSOFTCHECK(rclc_executor_spin_some(&executor, RCL_MS_TO_NS(100)));
    temporizadormsg = ahora;
  }
  
  if(!apagado){
    if(ahora-temporizadorsensores >= 10){
      // Debido a que los sensores miden la presión relativa respecto a la de su entorno, las presiones medidas serán positivas.
      Presion_vacio = -mapFloat(analogRead(SENSORVACIO),500,valor0,-1.0,0.0); 
      // La presión en la Parte Blanda será la diferencia entre su presión respecto a la cámara menos la de la cámara respecto al exterior.
      Presion_blanda = mapFloat(analogRead(SENSORBLANDO),500,valor0,-1.0,0.0) + Presion_vacio; 
      
      if(Presion_vacio >= -0.45 && bombaactiva == false){
        bombaactiva = true;
      }
      else if(Presion_vacio <= -0.50 && bombaactiva == true){
        bombaactiva = false;
      }
      int estadobomba = (bombaactiva) ? HIGH:LOW;
      digitalWrite(BOMBA,estadobomba);
      
      if(Presion_vacio >= -0.3){
        valvula_blanda_activa = false;
        valvula_camara_activa = false;
      }
      else{
        if(valorControl <= -0.2){
          valvula_blanda_activa = false;
          if(Presion_blanda >= valorControl+0.3 && valvula_camara_activa == false){
            valvula_camara_activa = true;
          } else if(Presion_blanda <= valorControl-0.1 && valvula_camara_activa == true){
            valvula_camara_activa = false;
          } 
          if(valvula_camara_activa){
            digitalWrite(VALVULA_BLANDA,LOW);
            digitalWrite(VALVULA_CAMARA,HIGH);
          } else {
            digitalWrite(VALVULA_BLANDA,LOW);
            digitalWrite(VALVULA_CAMARA,LOW);
          }
          
        } else {
          if(valvula_blanda_activa == false && Presion_blanda <= -0.2){
            valvula_blanda_activa = true;
            contador = ahora;
          }
        } 
      }
      if(valvula_blanda_activa == true){
        digitalWrite(VALVULA_BLANDA,HIGH);
        digitalWrite(VALVULA_CAMARA,LOW);
        
      } else {
        digitalWrite(VALVULA_BLANDA,LOW);
      }    
      temporizadorsensores = ahora;
    }
    if(ahora - contador >= 2500 && valvula_blanda_activa == true){
      valvula_blanda_activa = false;
      digitalWrite(VALVULA_BLANDA,LOW);
    }
  }
}
