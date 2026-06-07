//Definicion de pines
#define SENSORBLANDO 12
#define SENSORVACIO 13
#define BOMBA 25
#define VALVULA_BLANDA 26  // En los cables de la placa con los relays, el cable blanco es el destinado al relay de la izquierda mientras que el amarillo es el de la derecha
#define VALVULA_CAMARA 27

// Variables
float valorControl = 0.0;
unsigned long valor0 = 0;

const int tamano = 4000;
float presionVacio[tamano];
float presionBlando[tamano];

float mapFloat(float value, float fromLow, float fromHigh, float toLow, float toHigh) {
  return (value - fromLow) * (toHigh - toLow) / (fromHigh - fromLow) + toLow; 
}

void setup() 
{
  pinMode(VALVULA_BLANDA,OUTPUT); 
  pinMode(VALVULA_CAMARA,OUTPUT); 
  pinMode(BOMBA,OUTPUT);
  Serial.begin(115200);
  
  delay(2000);
  Serial.println("Empieza la prueba.");

  digitalWrite(VALVULA_BLANDA,HIGH);
  digitalWrite(VALVULA_CAMARA,LOW);
  delay(3000);
  valor0 = analogRead(SENSORBLANDO)+10;
  delay(1000);
  digitalWrite(VALVULA_BLANDA,LOW);
  digitalWrite(VALVULA_CAMARA,LOW);

}

unsigned long ms = 0;
unsigned long ahora = 0;
unsigned long contador2 = 0;
unsigned long n = 0;
unsigned long i = 0;
bool terminado = false;
bool mostrado = false;
unsigned long contador = 0;
bool valvulablandaactiva = false;
bool valvulacamaraactiva = false;
bool bombaactiva = false;
bool recorrido2 = false;

void loop() 
{
  ahora = millis();

  if(ahora - ms >= 5 && terminado == false){
    presionVacio[n] = mapFloat(analogRead(SENSORVACIO),500,valor0,-1.0,0.0); // Con una resistencia de 120 Ohms nos va a dar la presión en bares entre -1 y 1.   
    presionBlando[n] = mapFloat(analogRead(SENSORBLANDO),500,valor0,-1.0,0.0); // La mejor resistencia sería de 240 Ohms y mediríamos solo la presión entre -1 y 0.

    if(presionVacio[n] >= -0.50 && bombaactiva == false){
      bombaactiva = true;
    }
    else if(presionVacio[n] <= -0.55 && bombaactiva == true){
      bombaactiva = false;
    }
    int estadobomba = (bombaactiva) ? HIGH:LOW;
    digitalWrite(BOMBA,estadobomba);
    
    if(presionVacio[n] >= -0.1){        
        //valvulacamaraactiva = false;
        valvulablandaactiva = false;
        
    } else{
      if(valorControl <= -0.2){
        if(presionVacio[n]<-0.45){
          valvulablandaactiva = false;
          if(presionBlando[n] >= -0.35 && valvulacamaraactiva == false){
            valvulacamaraactiva = true;
            contador2 = ahora;
          } else if(presionBlando[n] <= -0.55 && valvulacamaraactiva == true && ahora - contador2 >= 500){
            valvulacamaraactiva = false;
          } 
        }        
      } else {
        
        if(valvulablandaactiva == false && presionBlando[n] <= -0.2){
          valvulacamaraactiva = false;
          valvulablandaactiva = true;
          contador = ahora;
        }
      } 
    }
    
    if(valvulablandaactiva == true){
      digitalWrite(VALVULA_BLANDA,HIGH);
           
    } else {
      digitalWrite(VALVULA_BLANDA,LOW);
    }    
    if(ahora - contador >= 2500 && valvulablandaactiva == true){
      valvulablandaactiva = false;
      digitalWrite(VALVULA_BLANDA,LOW);
    }
    
    int estadoCamara = (valvulacamaraactiva) ? HIGH:LOW;
    digitalWrite(VALVULA_CAMARA,estadoCamara);
/*
    if(ahora - contador2 >= 500 && valvulacamaraactiva == true){
      valvulacamaraactiva = false;
      digitalWrite(VALVULA_CAMARA,LOW);
    }*/
  
    n++;
    ms = ahora;

    if(n<=1000){
      valorControl = 0.0;
    } else if(n<=1600){
      valorControl = -0.45;
    } else if(n<= 2400){
      valorControl = 0.0;
    } else if(n<=3000){
      valorControl = -0.45;
    } else{
      valorControl = 0.0;
    }
  }

  if(n>=tamano && mostrado == false){
    terminado = true;
    digitalWrite(VALVULA_CAMARA,LOW);
    digitalWrite(VALVULA_BLANDA,LOW);
    digitalWrite(BOMBA,LOW);

    Serial.print(i);
    Serial.print("   ");
    Serial.print(presionVacio[i]);
    Serial.print("   ");
    Serial.print(presionBlando [i]);
    Serial.print("   ");
    
    if(i<=1000){
      Serial.println(0.0);
    } else if(i<=1600){
      Serial.println(-0.45);
    } else if(i<= 2400){
      Serial.println(0.0);
    } else if(i<=3000){
      Serial.println(-0.45);
    } else{
      Serial.println(0.0);
    }
    i++;
  }

  if(i>=tamano){
    n = 0;
    i = 0;
    terminado = false;
  }
}
