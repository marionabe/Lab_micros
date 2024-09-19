#include <avr/io.h>
#include <util/delay.h>
#include <stdlib.h>
#include <time.h>
#include <avr/interrupt.h>


void encender_leds();


  /* Codificación de estados:
     Nombre del estado                   |  Numero del estado
     ____________________________________|____________________
   */
    #define Configurar_todo                         1
    #define No_se_ha_iniciado_el_juego              2
    #define Se_ha_encendido_1_led                   3
    #define Se_han_encendido_2_led                  4
    #define Se_han_encendido_3_led                  5
    #define Se_han_encendido_4_led                  6
    #define No_se_ha_presionado_ningun_boton        7
    #define Se_ha_presionado_un_boton               8
    #define Se_ha_perdido_el_juego                  9
    #define Se_ha_presionado_un_boton_correcto      10
    #define Se_ha_ganado_el_juego                   11




//Esta variable se utiliza como otro preescalar para los timers
int counter_int=0;

int counter_desbloque_de_clic;


//Esta variable se utiliza para llevar en registro el orden en que se encienden los leds
int secuencia_de_leds[4];

// Para llevar el registro de la dificultad del juego
int dificultad = 1;
int boton_correcto         = 0;
int algun_boton_presionado = 0;
int estado_actual          = 1;
int sig_estado             = 1;
int num_btn_presionados    = 0;
int num_leds_encedidos     = 0;

int pinD6_anterior;
int pinD5_anterior;
int pinD4_anterior;
int pinD3_anterior;

int pinD6_actual;
int pinD5_actual;
int pinD4_actual;
int pinD3_actual;


int main(void)
{
    estado_actual=Configurar_todo;
    int cont_de_ciclos=0;

  while (1) {
      cont_de_ciclos++;
      if (cont_de_ciclos==100){
          cont_de_ciclos=0;
      }
      srand(cont_de_ciclos);

      switch (estado_actual){
          case Configurar_todo:
              DDRB = 0xAA; //Configuracion de puertos como entrada o salida
              
              sei();    //Habilitar interrupciones globales
              GIMSK = 0X10; //habilitar la interrupcion PCIE0 para los botones
              PCMSK2 = 0x78; //activar la interrupcion del pin b3 usando una mascara
              
              TIMSK = 0X40; 
              TCNT1 = 0x0; //establecer el contador del timer en cero.
              OCR1A = 0xFF; //Se establece el valor a comparar
              TCCR1B |= 0x05; // cofig para usar el clock con el mayor escalar
              
              dificultad = 1;
              boton_correcto         = 0;
              algun_boton_presionado = 0;
              num_btn_presionados    = 0;
              num_leds_encedidos     = 0;
              
              sig_estado = No_se_ha_iniciado_el_juego;
              break;

      case No_se_ha_iniciado_el_juego:
              if (algun_boton_presionado){
                  sig_estado = Se_ha_encendido_1_led;
              } else {
                  sig_estado = No_se_ha_iniciado_el_juego;
              }
              break;
      case Se_ha_encendido_1_led:
              encender_leds();          
              algun_boton_presionado=0;
              
              if (dificultad==1){
                  sig_estado = No_se_ha_presionado_ningun_boton;
                  algun_boton_presionado=0;
              }else {
                  sig_estado = Se_han_encendido_2_led;
              }
              break;
              
      case Se_han_encendido_2_led:
              if ((PORTB & 0X55)==0X0){ //Verificar que todos los leds se han apagado antes de continuar
                  encender_leds();
                  if (dificultad==2){
                      sig_estado = No_se_ha_presionado_ningun_boton;
                      algun_boton_presionado=0;
                  }else{
                      sig_estado = Se_han_encendido_3_led;
                  }
              }
              break;
      case Se_han_encendido_3_led:
              if ((PORTB & 0X55)==0X0){
                  encender_leds();
                  //Encender el tercer led, diferente a los otros
                  //guardar cuál led se encendió
                  if(dificultad==3){
                      sig_estado = No_se_ha_presionado_ningun_boton;
                      algun_boton_presionado=0;
                  }else {
                      sig_estado = Se_han_encendido_4_led;
                  }
              }
              break;
      case Se_han_encendido_4_led:
              if ((PORTB & 0X55)==0X0){
                  encender_leds();
                  //Se enciende el cuarto led, el ultimo
                  //Se guarda cuál fue el led que se encendió
                  sig_estado = No_se_ha_presionado_ningun_boton;
                  algun_boton_presionado=0;
              }
              break;
      case No_se_ha_presionado_ningun_boton:
              // En este estado solo se espera un clik
              if (algun_boton_presionado){
                  sig_estado = Se_ha_presionado_un_boton;
                  algun_boton_presionado=0;
              }else{
                  sig_estado = No_se_ha_presionado_ningun_boton;
              }
              break;
      case Se_ha_presionado_un_boton:
              if (boton_correcto==1){
                  sig_estado=Se_ha_presionado_un_boton_correcto;
                  boton_correcto=0;
              }else{
                  sig_estado=Se_ha_perdido_el_juego;
              }
              break;
      case Se_ha_perdido_el_juego:
              dificultad=1;
              num_btn_presionados = 0;
              num_leds_encedidos = 0;
              secuencia_de_leds[0]=0;
              secuencia_de_leds[1]=0;
              secuencia_de_leds[2]=0;
              secuencia_de_leds[3]=0;
              algun_boton_presionado=0;
              
              PORTB = 0XFF;
              counter_int=0;
              TCNT1 = 0x0;

              PORTB = 0XFF;
              counter_int=0;
              TCNT1 = 0x0;

              PORTB = 0XFF;
              counter_int=0;
              TCNT1 = 0x0;

              sig_estado = No_se_ha_iniciado_el_juego;
              break;

      case Se_ha_presionado_un_boton_correcto:
              if (num_btn_presionados==num_leds_encedidos){
                  sig_estado = Se_ha_ganado_el_juego;
              }else{
                  sig_estado = No_se_ha_presionado_ningun_boton;
              }
              break;
      case Se_ha_ganado_el_juego:
              dificultad++;
              num_btn_presionados = 0;
              num_leds_encedidos = 0;
              secuencia_de_leds[0]=0;
              secuencia_de_leds[1]=0;
              secuencia_de_leds[2]=0;
              secuencia_de_leds[3]=0;
              
              sig_estado = Se_ha_encendido_1_led;
      default:
      break;
  }
  estado_actual=sig_estado;
  pinD3_anterior = PIND &(1<<PIND3);
  pinD4_anterior = PIND &(1<<PIND4);
  pinD5_anterior = PIND &(1<<PIND5);
  pinD6_anterior = PIND &(1<<PIND6);
  }
}


void encender_leds (){
    int led_a_encender=1;
    int led_ya_encendido=1;

    do{
        led_a_encender = 1 + rand() % (4 - 1 + 1);
        if (led_a_encender != secuencia_de_leds[0] &&
            led_a_encender != secuencia_de_leds[1] &&
            led_a_encender != secuencia_de_leds[2] &&
            led_a_encender != secuencia_de_leds[3] ){
            led_ya_encendido=0;
        }else{
            led_ya_encendido=1;
        }
    }while (led_ya_encendido==1);
    num_leds_encedidos++;

    //Ir almacenando el numero del led que se encendió
    secuencia_de_leds[num_leds_encedidos-1]=led_a_encender;

    switch (led_a_encender){
        case 1:
            PORTB |= 0X01;
            break;
        case 2:
            PORTB |= 0X04;
            break;
        case 3:
            PORTB |= 0X10;
            break;
        case 4:
            PORTB |= 0X40;
            break;
        default:
            PORTB = 0xFF;
            break;
        }
    counter_int=0;
    TCNT1 = 0x0; //establecer el contador del timer en cero.
}


ISR (PCINT2_vect){
    algun_boton_presionado=1;
    GIMSK = 0X00; //Deshabilitar las interrupciones temporalmente
    pinD3_actual= PIND &(1<<PIND3);
    pinD4_actual= PIND &(1<<PIND4);
    pinD5_actual= PIND &(1<<PIND5);
    pinD6_actual= PIND &(1<<PIND6);

    if (estado_actual == No_se_ha_presionado_ningun_boton){    
        num_btn_presionados++;
        if (pinD3_actual != pinD3_anterior){
            if (secuencia_de_leds[num_btn_presionados-1]==3){
                boton_correcto=1;
            }
        }else if(pinD4_actual != pinD4_anterior){
            if (secuencia_de_leds[num_btn_presionados-1]==4){
                boton_correcto=1;
            }
        }else if (pinD5_actual != pinD5_anterior){
            if (secuencia_de_leds[num_btn_presionados-1]==2){
                boton_correcto=1;
            }
        }else if (pinD6_actual != pinD6_anterior){
            if (secuencia_de_leds[num_btn_presionados-1]==1){
                boton_correcto=1;
            }
        }else{
            boton_correcto=0;
        }    
    }
}


ISR (TIMER1_COMPA_vect){
    counter_int++;
    counter_desbloque_de_clic++;
    if (counter_int == 60-(dificultad*5)){
        PORTB= 0x0;
        counter_int=0;
    }
    if (counter_desbloque_de_clic==15){
        GIMSK = 0X10; // Volver a habilitar las interrupciones PCINT
        counter_desbloque_de_clic=0;
    }
    TCNT1 = 0x0; //establecer el contador del timer en cero.
}
