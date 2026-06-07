import sys
import rclpy
import math
from rclpy.node import Node
from PyQt5.QtWidgets import (
    QApplication, QMainWindow, QVBoxLayout, QWidget, 
    QLabel, QSlider, QPushButton,QHBoxLayout,QMessageBox
)
from PyQt5.QtCore import QTimer, Qt 
from std_msgs.msg import Int32, Float32, String

# ----------------------------------------------------------------------
# CLASE PRINCIPAL DE LA GUI
# ----------------------------------------------------------------------
class ROS2_GUI_Window(QMainWindow):
    def __init__(self):
        super().__init__()
        self.setWindowTitle("Panel de control de Jamming")
        self.setGeometry(100, 100, 450, 450)

        # Inicialización de ROS 2 y GUI
        self.setup_ui()
        self.setup_ros2()
        
    # --- CONFIGURACIÓN DE LA GUI ---
    def setup_ui(self):
        self.is_off = False

        self.central_widget = QWidget()
        self.setCentralWidget(self.central_widget)
        self.layout = QVBoxLayout(self.central_widget)

        self.top_visual_layout = QHBoxLayout()
        
        self.bend_stick = BendingStickWidget()
        self.bend_stick.setFixedSize(300,250)

        self.color_changing = VacuumChamberWidget()
        self.color_changing.setFixedSize(200, 250)
        
        self.top_visual_layout.addWidget(self.bend_stick)
        self.top_visual_layout.addWidget(self.color_changing)


        self.middle_visual_layout = QHBoxLayout()
        self.presion_blanda = QLabel("0.0 bar")
        self.presion_blanda.setAlignment(Qt.AlignCenter)
        self.presion_blanda.setStyleSheet("""
            font-size: 16pt; 
            font-weight: bold; 
            color: #2c3e50;
            margin-top: 10px;
            margin-bottom: 5px;
        """)

        self.presion_camara = QLabel("0.0 bar")
        self.presion_camara.setAlignment(Qt.AlignCenter)
        self.presion_camara.setStyleSheet("""
            font-size: 16pt; 
            font-weight: bold; 
            color: #2c3e50;
            margin-top: 10px;
            margin-bottom: 5px;
        """)
        self.middle_visual_layout.addWidget(self.presion_blanda)
        self.middle_visual_layout.addWidget(self.presion_camara)
        
        # 2. CONTROL CONTINUO (Deslizador)
        self.slider = QSlider(Qt.Horizontal)
        self.slider.setRange(0, 1) 
        self.slider.setValue(0)
        self.slider.valueChanged.connect(self.publish_command) 

        # Botón de Apagar
        self.off_button = QPushButton("⏻")
        self.off_button.setFixedSize(50, 50) 
        
        self.off_button.setStyleSheet("""
            QPushButton {
                background-color: #ff1744;
                color: white;
                font-size: 24px;
                border-radius: 25px; /* Mitad de 50 para que sea circular */
                border: 2px solid #b71c1c;
            }
            QPushButton:hover {
                background-color: #d50000;
            }
            QPushButton:pressed {
                background-color: #880e4f;
            }
        """)
        self.off_button.clicked.connect(self.publish_shutdown)
        
        # # 3. CONTROL DISCRETO (Botón de duro)
        # self.hard_button = QPushButton("Duro")
        # self.hard_button.setStyleSheet("background-color: green; color: black; font-weight: bold; padding: 10px;")
        # self.hard_button.clicked.connect(self.publish_hard_command)
        # # Botón de blando
        # self.soft_button = QPushButton("Blando")
        # self.soft_button.setStyleSheet("background-color: blue; color: black; font-weight: bold; padding: 10px;")
        # self.soft_button.clicked.connect(self.publish_soft_command)
        self.buttons_layout = QHBoxLayout() # Creamos el layout horizontal
        
        self.hard_button = QPushButton("ENDURECER")
        self.hard_button.setStyleSheet("background-color: green; color: black; font-weight: bold;font-size: 18px; padding: 30px;")
        self.hard_button.clicked.connect(self.publish_hard_command)
        
        self.soft_button = QPushButton("ABLANDAR")
        self.soft_button.setStyleSheet("background-color: blue; color: black; font-weight: bold;font-size: 18px; padding: 30px;")
        self.soft_button.clicked.connect(self.publish_soft_command)
        
        # Añadimos los botones al layout horizontal
        self.buttons_layout.addWidget(self.soft_button) # Izquierda
        self.buttons_layout.addWidget(self.hard_button) # Derecha
        # Añadir todos los widgets al layout
        #self.layout.addWidget(self.shape_display, alignment=Qt.AlignCenter)
        #self.layout.addWidget(QLabel("Sistema embebido de robótica blanda:"),alignment=Qt.AlignCenter)
        #self.layout.addStretch(1) # Añade espacio elástico arriba
        #self.layout.addWidget(self.bend_stick,alignment=Qt.AlignCenter)
        self.layout.addLayout(self.top_visual_layout)
        #self.layout.addWidget(QLabel("Porcentaje de dureza de la parte blanda:"))
        #self.layout.addWidget(self.value_label)
        self.layout.addLayout(self.middle_visual_layout)
        self.layout.addWidget(self.slider)
        self.layout.addLayout(self.buttons_layout)
        self.layout.addWidget(self.off_button,alignment=Qt.AlignCenter)
        # self.layout.addWidget(self.hard_button)
        # self.layout.addWidget(self.soft_button)
        


    # --- INTEGRACIÓN ROS 2 ---
    def setup_ros2(self):
        # 1. Creación del nodo ROS 2
        self.node = rclpy.create_node('gui_control_panel')
        self.node.get_logger().info('ROS 2 Node Initialized.')
        
        # 2. Publicadores
        #self.vel_publisher = self.node.create_publisher(Float32, "/control_command", 10)
        self.state_publisher = self.node.create_publisher(Int32, "/ControlTeclado", 10)

        self.shutdown_publisher = self.node.create_publisher(Int32, "/Apagar", 10)

        # 3. Suscripción (para cambiar el color de la forma)
        self.parte_blanda = self.node.create_subscription(
            Float32, 
            '/ParteBlanda', 
            self.parte_blanda_callback, 
            10 
        )

        self.camara_vacio = self.node.create_subscription(
            Float32, # O Float32 según tus mensajes
            '/CamaraVacio', 
            self.camara_vacio_callback, 
            10 
        )
        
        # 4. Configurar el QTimer para el spinning no-bloqueante
        self.timer = QTimer()
        self.timer.timeout.connect(self.ros2_spin) 
        self.timer.start(33) # 30 Hz

    def ros2_spin(self):
        #Llama a spin_some() para procesar callbacks pendientes.
        if self.node:
            rclpy.spin_once(self.node,timeout_sec=0)
            
    # --- CALLBACKS Y PUBLICACIÓN ---
    def parte_blanda_callback(self, msg):
        #"""Recibe el número de ROS 2 y actualiza el color de la forma."""
        parte_blanda_presion = msg.data
        # style_template = "border: 3px solid black; border-radius: 10px;"

        # self.node.get_logger().warn(f"El numero recibido es: {parte_blanda_presion}")

        self.presion_blanda.setText(f"{parte_blanda_presion:.2f} bar")

        # if parte_blanda_presion <= -0.3:
        #     self.slider.setValue(1)
        # else:
        #     self.slider.setValue(0)
        # if parte_blanda_presion == 1:
        #     self.shape_display.setStyleSheet(f"background-color: blue; {style_template}") 
            
        # elif parte_blanda_presion == 0:
        #     self.shape_display.setStyleSheet(f"background-color: red; {style_template}")
            
        # else:
        #     self.shape_display.setStyleSheet(f"background-color: gray; {style_template}")
        # self.shape_display.update()
        self.bend_stick.set_bend_value(int(52*(1-math.exp(parte_blanda_presion*5))))
    
    def camara_vacio_callback(self,msg):
        camara_presion = msg.data

        self.color_changing.set_pressure_value(camara_presion)
        self.presion_camara.setText(f"{camara_presion:.2f} bar")

    def publish_command(self, value):
        #"""Función conectada al QSlider para publicar el comando de velocidad."""
        valor = int(value)*100
        msg = Int32() 
        msg.data = valor
        self.state_publisher.publish(msg) 
        self.node.get_logger().info(f'Publicando dureza del {valor}%')

    def publish_hard_command(self):
        #"""Función conectada al QPushButton para publicar el paro."""
        msg = Int32() 
        msg.data = 100
        self.state_publisher.publish(msg) 
        self.node.get_logger().warn('Publicando valor de maxima dureza')
        self.slider.setValue(1)

    def publish_soft_command(self):
        #"""Función conectada al QPushButton para publicar el paro."""
        msg = Int32() 
        msg.data = 0
        self.state_publisher.publish(msg) 
        self.node.get_logger().warn('Publicando valor de totalmente blando')
        self.slider.setValue(0)
    
    def publish_shutdown(self):
        self.is_off = not self.is_off

        reply = QMessageBox.question(self, 'Confirmar', "¿Deseas apagar el sistema?",
                                QMessageBox.Yes | QMessageBox.No, QMessageBox.No) if self.is_off else QMessageBox.question(self, 'Confirmar', "¿Deseas encender el sistema?",
                                QMessageBox.Yes | QMessageBox.No, QMessageBox.No) 
        
        if reply == QMessageBox.Yes:
            # 2. Definir los colores según el estado
            # Si está en "OFF", se pone rojo brillante, si no, un rojo oscuro o gris
            color_fondo = "#01708B" if self.is_off else "#ff1744"
            color_borde = "#01496B" if self.is_off else "#b71c1c" 
            
            # 3. Aplicar el nuevo estilo manteniendo la forma circular
            self.off_button.setStyleSheet(f"""
                QPushButton {{
                    background-color: {color_fondo};
                    color: white;
                    font-size: 24px;
                    border-radius: 25px;
                    border: 2px solid {color_borde};
                }}
                QPushButton:hover {{
                    background-color: {color_borde};
                }}
            """)
            msg = Int32()
            msg.data = 1 # Mensaje de comando
            self.shutdown_publisher.publish(msg)
            self.node.get_logger().warn("Enviando señal de APAGADO/ENCENDIDO")

    # --- LIMPIEZA ---
    def closeEvent(self, event):
        #"""Función llamada cuando la ventana se cierra."""
        self.timer.stop()
        if self.node:
            self.node.destroy_node()
        event.accept()




from PyQt5.QtWidgets import QWidget
from PyQt5.QtGui import QPainter, QColor, QPen, QPainterPath,QLinearGradient
from PyQt5.QtCore import Qt, QPoint, QRectF

class BendingStickWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._bend_value = 0 # Valor de 0 a 100
        self.setMinimumSize(300, 200) # Tamaño mínimo para tener espacio para dibujar

    # 1. El método que dibuja
    def paintEvent(self, event):
        # Aquí vamos a dibujar nuestro palo curvado
        painter = QPainter(self)

        # 1. Configuración del Pincel (El palo)
        painter.setRenderHint(QPainter.Antialiasing) # Suavizado
        pen = QPen(QColor(0, 0, 0), 70) 
        #pen.setCapStyle(Qt.RoundCap) # Extremos redondeados
        painter.setPen(pen)

        # 2. Geometría Fija
        width = self.width()
        height = self.height()
        
        # Punto A (Base Fija del Palo): Centro Inferior
        point_A1 = QPoint(int(width / 2)+20, height - 10)
        point_A2 = QPoint(int(width / 2)-20, height - 10)
                
        # Punto B (Cima Fija del Palo): Centro Superior
        point_B1 = QPoint(int(width / 2)+20, int(height/2) - 40)
        point_B2 = QPoint(int(width / 2)-20, int(height/2) - 40)

        # 3. Punto de Control Dinámico (La Flexión)
        max_horizontal_bend = 30 # Máximo desplazamiento horizontal (pixels)

        # La flexión debe ser máxima (80px) cuando _bend_value es 0
        # y cero (0px) cuando _bend_value es 100.
        
        horizontal_offset = max_horizontal_bend*(1-self._bend_value/50) 
        if horizontal_offset <= 0:
            horizontal_offset = 0
        
        # La coordenada X del control point se mueve para crear la curva
        control_point_X1 = int(point_A1.x() + horizontal_offset/2)
        control_point_X2 = int(point_A2.x() + horizontal_offset)
        # La coordenada Y está en el centro entre A y B
        control_point_Y1 = int( 37 + pow(horizontal_offset,2)/100)
        control_point_Y2 = int(pow(horizontal_offset,2)/100)

        # if control_point_Y2<=37:
        #     control_point_Y2 = 37
        
        control_point1 = QPoint(control_point_X1, control_point_Y1)
        control_point2 = QPoint(control_point_X2, control_point_Y2)

        # 4. Creación y Dibujo del Path
        path = QPainterPath(point_A1)
        path.quadTo(point_B1, control_point1)

        path2 = QPainterPath(point_A2)
        path2.quadTo(point_B2, control_point2)
        
        painter.drawPath(path)
        #painter.drawPath(path2)
        
        painter.end() 

    # 2. Método para actualizar el valor desde ROS 2
    def set_bend_value(self, value):
        if 0 <= value <= 100:
            self._bend_value = value
            self.update() # Forzamos la llamada a paintEvent

class VacuumChamberWidget(QWidget):
    def __init__(self, parent=None):
        super().__init__(parent)
        self._pressure = 0.0  # Rango de 0.0 a -0.5
        self.setMinimumSize(120, 250)

    def set_pressure_value(self, value):
        # Limitamos al rango de tu sensor
        self._pressure = max(-0.65, min(0.0, value))
        self.update()

    def paintEvent(self, event):
        painter = QPainter(self)
        painter.setRenderHint(QPainter.Antialiasing)

        # Calculamos un ratio de intensidad (0.0 en ambiente, 1.0 en -0.5 bares)
        # Usamos abs() porque el valor es negativo
        intensity = abs(self._pressure) / 0.6 

        width = self.width()
        height = self.height()
        
        # --- GEOMETRÍA DINÁMICA (Efecto Succión) ---
        # En presión ambiente el ancho es 70, en vacío total se contrae a 50
        base_width = 80
        contracted_width = base_width - (20 * intensity)
        
        col_x = (width - contracted_width) / 2
        col_y = 20
        col_h = height - 20

        # --- COLOR DINÁMICO (Efecto Vacío) ---
        # De Gris azulado (Ambiente) a Azul Profundo (Vacío)
        r = int(200 * (1 - intensity) + 30 * intensity)
        g = int(210 * (1 - intensity) + 40 * intensity)
        b = int(220 * (1 - intensity) + 150 * intensity)
        
        main_color = QColor(r, g, b)

        # Dibujamos la forma con bordes redondeados
        rect = QRectF(col_x, col_y, contracted_width, col_h)
        radius = contracted_width / 8

        painter.setPen(QPen(QColor(50, 50, 50), 2))
        painter.setBrush(main_color)
        painter.drawRoundedRect(rect, radius, radius)

        painter.end()


# ----------------------------------------------------------------------
# FUNCIÓN PRINCIPAL DE EJECUCIÓN
# ----------------------------------------------------------------------
def main(args=None):
    # Inicializar ROS 2 primero
    rclpy.init(args=args) 
    
    # Inicializar la aplicación de PyQt
    app = QApplication(sys.argv)
    window = ROS2_GUI_Window()
    window.show()
    
    # Ejecutar el bucle de la GUI. El nodo ROS 2 se ejecuta en paralelo vía QTimer.
    sys.exit(app.exec_())

if __name__ == '__main__':
    main()
    rclpy.shutdown() # Asegurarse de que ROS 2 se apague limpiamente