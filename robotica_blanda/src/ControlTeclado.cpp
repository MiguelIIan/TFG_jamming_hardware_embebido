#include <rclcpp/rclcpp.hpp>
#include "std_msgs/msg/int32.hpp"
#include "std_msgs/msg/string.hpp"
#include <signal.h>
#include <termios.h>
#include <stdio.h>
#include <unistd.h>

#define BLANDO 0x62
#define VACIO 0x76
#define SALIR 0x71

class Teclado : public rclcpp::Node
{
public:

    Teclado(): Node ("controlteclas"){
        // Definición del tópico del publicador
        tecla = this->create_publisher<std_msgs::msg::Int32>("/ControlTeclado", 1);
    }

    void keyLoop()
    {
        char c;
        bool dirty = false;

        // Obtener la terminal en modo directo
        tcgetattr(kfd, &cooked);
        memcpy(&raw, &cooked, sizeof(struct termios));
        raw.c_lflag &= ~(ICANON | ECHO);

        // Definiendo una nueva línea e instrucciones para el usuario
        raw.c_cc[VEOL] = 1;
        raw.c_cc[VEOF] = 2;
        tcsetattr(kfd, TCSANOW, &raw);
        puts("Leyendo del teclado");
        puts("---------------------------");
        puts("Pulse v para endurecer el eslabón");
        puts("Pulse b para ablandecer el eslabón");
        puts("Pulse q para salir del programa");

        // Bucle infinito
        while (rclcpp::ok())
        {
            // Obtener la próxima tecla pulsada
            if (read(kfd, &c, 1) < 0)
            {
                perror("read():");
                exit(-1);
            }

            // Crear la variable mensaje de la comunicación ROS2
            std_msgs::msg::Int32 mensaje;
            
            // Switch para la actuación en función de la tecla pulsada
            switch (c)
            {
            case VACIO:
                RCLCPP_DEBUG(this->get_logger(), "VACIO");
                valor = 100;
                dirty = true;
                break;
            case BLANDO:
                RCLCPP_DEBUG(this->get_logger(), "BLANDO");
                valor = 0;
                dirty = true;
                break;
            case SALIR:
                RCLCPP_DEBUG(this->get_logger(), "SALIR");
                RCLCPP_INFO(this->get_logger(), "El control por teclado a finalizado");
                return;
                break;
            }

            // Se guarda el valor en el mensaje de ROS2
            mensaje.data = valor;

            // El mensaje se publica si y solo si se ha pulsado alguna de las teclas predefinidas
            if (dirty == true)
            {
                tecla->publish(mensaje);
                dirty = false;
            }
        }

        return;
    }

    void quit()
    {

        tcsetattr(kfd, TCSANOW, &cooked);
        exit(0);
    }

private:
    //Definición del publicador y de las variables necesarias
    rclcpp::Publisher<std_msgs::msg::Int32>::SharedPtr tecla;
    int kfd = 0;
    int valor = 0;
    struct termios cooked, raw;
};

// MAIN
int main(int argc, char* argv[])
{
    // Inicialización del nodo de ROS2
    rclcpp::init(argc, argv);

    // Creación del objeto de la clase "Teclado"
    auto myNode = std::make_shared<Teclado>();
    
    // Bucle del nodo
    myNode->keyLoop();
    myNode->quit();

    // Apagado del nodo
    rclcpp::shutdown();
    return 0;
}