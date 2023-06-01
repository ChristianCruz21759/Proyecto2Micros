
# Importar librerias
from PyQt5 import QtCore, QtGui, QtWidgets
from Adafruit_IO import Client, Feed, Data, RequestError
import datetime, serial, time

ADAFRUIT_IO_KEY = 'aio_NYfn09JeXXKBb9180OKu6qlZw4kw'

ADAFRUIT_IO_USERNAME = 'Cruz21759'

aio = Client(ADAFRUIT_IO_USERNAME, ADAFRUIT_IO_KEY)
try:
    base = aio.feeds('brazo-mecanico.base')
    brazo = aio.feeds('brazo-mecanico.brazo')
    antebrazo = aio.feeds('brazo-mecanico.antebrazo')
    garra = aio.feeds('brazo-mecanico.garra')
except RequestError:
    print("error")
    
with serial.Serial() as ser:    # EUSART config
    ser.baudrate = 9600
    ser.port = 'COM4'

def receive_data ():            # Funcion para recibir los datos de adafruit y enviarlos por serial    
                             
    data_base = int(aio.receive(base.key).value)  # Recibir datos de adafruit base
    send_data(data_base)        # Enviar                    
    print(data_base)
    time.sleep(1)
    
    data_brazo = int(aio.receive(brazo.key).value)  # Recibir datos de adafruit brazo
    send_data(data_brazo)       # Enviar
    print(data_brazo)
    time.sleep(1)
    
    data_antebrazo = int(aio.receive(antebrazo.key).value)  # Recibir datos de adafruit antebrazo
    send_data(data_antebrazo)   # Enviar
    print(data_antebrazo)
    time.sleep(1)
    
    data_garra = int(aio.receive(garra.key).value)  # Recibir datos de adafruit garra
    ser.open()
    ser.write(bytes(str(data_garra), 'utf-8'))      # Enviar
    ser.close()
    print(data_garra)
    time.sleep(1)
    
def send_data (data):           # Funcion parar enviar los datos en digitos
    
    ser.open()
    
    if data < 100:                              # Centenas = 0
        ser.write(b'0')
    if data < 10:                               # Decenas = 0
        ser.write(b'0')
    
    ser.write(bytes(str(data), 'utf-8'))        # Escribir el valor al puerto serial

    ser.close()

class Ui_MainWindow(object):    # Clase creada por Qt Designer para la interfaz
    def setupUi(self, MainWindow):
        MainWindow.setObjectName("MainWindow")
        MainWindow.resize(500, 200)
        self.centralwidget = QtWidgets.QWidget(MainWindow)
        self.centralwidget.setObjectName("centralwidget")
        
        # Slider Base
        self.slider1 = QtWidgets.QSlider(self.centralwidget)
        self.slider1.setGeometry(QtCore.QRect(140, 50, 171, 22))
        self.slider1.setOrientation(QtCore.Qt.Horizontal)
        self.slider1.setObjectName("slider1")
        self.slider1.setMinimum(0)
        self.slider1.setMaximum(100)
        self.slider1.setEnabled(False)
        self.slider1.valueChanged.connect(self.slider1_fun)         # Si el valor cambia conectar a una funcion
        self.slider1.sliderReleased.connect(self.send_eusart)       # Si el slider se suelta conectar a otra funcion
        
        # Slider Brazo
        self.slider2 = QtWidgets.QSlider(self.centralwidget)
        self.slider2.setGeometry(QtCore.QRect(140, 80, 171, 22))
        self.slider2.setOrientation(QtCore.Qt.Horizontal)
        self.slider2.setObjectName("slider2")
        self.slider2.setMinimum(0)
        self.slider2.setMaximum(100)
        self.slider2.setEnabled(False)
        self.slider2.valueChanged.connect(self.slider2_fun)
        self.slider2.sliderReleased.connect(self.send_eusart)
        
        # Slider Antebrazo
        self.slider3 = QtWidgets.QSlider(self.centralwidget)
        self.slider3.setGeometry(QtCore.QRect(140, 110, 171, 22))
        self.slider3.setOrientation(QtCore.Qt.Horizontal)
        self.slider3.setObjectName("slider3")
        self.slider3.setMinimum(0)
        self.slider3.setMaximum(100)
        self.slider3.setEnabled(False)
        self.slider3.valueChanged.connect(self.slider3_fun)
        self.slider3.sliderReleased.connect(self.send_eusart)
        
        #Boton Garra On/Off
        self.push5 = QtWidgets.QPushButton(self.centralwidget)
        self.push5.setGeometry(QtCore.QRect(180, 140, 93, 28))
        self.push5.setObjectName("push5")
        self.push5.setCheckable(True)
        self.push5.setChecked(False)
        self.push5.setEnabled(False)
        self.push5.clicked.connect(self.push_Garra)                 #Conectar a varias funciones cuando se presiona el boton
        self.push5.clicked.connect(self.send_eusart)
        
        # Label Contador Base 
        self.label1 = QtWidgets.QLabel(self.centralwidget)
        self.label1.setGeometry(QtCore.QRect(330, 50, 31, 16))
        self.label1.setObjectName("label1")
        
        # Labe2 Contador Brazo
        self.label2 = QtWidgets.QLabel(self.centralwidget)
        self.label2.setGeometry(QtCore.QRect(330, 80, 31, 16))
        self.label2.setObjectName("label2")
        
        # Labe3 Contador Antebrazo
        self.label3 = QtWidgets.QLabel(self.centralwidget)
        self.label3.setGeometry(QtCore.QRect(330, 110, 31, 16))
        self.label3.setObjectName("label3")
        
        # Boton Modo Manual
        self.push1 = QtWidgets.QPushButton(self.centralwidget)
        self.push1.setGeometry(QtCore.QRect(370, 20, 93, 28))
        self.push1.setObjectName("push1")
        self.push1.setCheckable(True)       # Boton con toggle
        self.push1.setChecked(False)        # Boton no presionado
        self.push1.clicked.connect(self.push_Manual)
        
        # Boton Modo EEPROM
        self.push2 = QtWidgets.QPushButton(self.centralwidget)
        self.push2.setGeometry(QtCore.QRect(370, 60, 93, 28))
        self.push2.setObjectName("push2")
        self.push2.setCheckable(True)
        self.push2.setChecked(False)
        self.push2.clicked.connect(self.push_EEPROM)
        
        # Boton Modo EUSART
        self.push3 = QtWidgets.QPushButton(self.centralwidget)
        self.push3.setGeometry(QtCore.QRect(370, 100, 93, 28))
        self.push3.setObjectName("push3")
        self.push3.setCheckable(True)
        self.push3.setChecked(False)
        self.push3.clicked.connect(self.push_EUSART)
        
        # Boton Modo adafruit io
        self.push4 = QtWidgets.QPushButton(self.centralwidget)
        self.push4.setGeometry(QtCore.QRect(370, 140, 93, 28))
        self.push4.setObjectName("push4")
        self.push4.clicked.connect(self.push_adafruit_io)
        
        
        self.label_4 = QtWidgets.QLabel(self.centralwidget)
        self.label_4.setGeometry(QtCore.QRect(30, 50, 55, 16))
        self.label_4.setObjectName("label_4")
        self.label_5 = QtWidgets.QLabel(self.centralwidget)
        self.label_5.setGeometry(QtCore.QRect(30, 80, 55, 16))
        self.label_5.setObjectName("label_5")
        self.label_6 = QtWidgets.QLabel(self.centralwidget)
        self.label_6.setGeometry(QtCore.QRect(30, 110, 71, 16))
        self.label_6.setObjectName("label_6")
        self.label_7 = QtWidgets.QLabel(self.centralwidget)
        self.label_7.setGeometry(QtCore.QRect(30, 140, 71, 16))
        self.label_7.setObjectName("label_7")
        self.label_8 = QtWidgets.QLabel(self.centralwidget)
        self.label_8.setGeometry(QtCore.QRect(30, 16, 241, 20))
        self.label_8.setTextFormat(QtCore.Qt.RichText)
        self.label_8.setObjectName("label_8")
        MainWindow.setCentralWidget(self.centralwidget)
        self.menubar = QtWidgets.QMenuBar(MainWindow)
        self.menubar.setGeometry(QtCore.QRect(0, 0, 500, 26))
        self.menubar.setObjectName("menubar")
        MainWindow.setMenuBar(self.menubar)
        self.statusbar = QtWidgets.QStatusBar(MainWindow)
        self.statusbar.setObjectName("statusbar")
        MainWindow.setStatusBar(self.statusbar)

        self.retranslateUi(MainWindow)
        QtCore.QMetaObject.connectSlotsByName(MainWindow)

    def retranslateUi(self, MainWindow):
        _translate = QtCore.QCoreApplication.translate
        MainWindow.setWindowTitle(_translate("MainWindow", "Proyecto2 - Panel de Control"))
        self.push5.setText(_translate("MainWindow", "Abierta"))
        self.label1.setText(_translate("MainWindow", "0"))
        self.label2.setText(_translate("MainWindow", "0"))
        self.label3.setText(_translate("MainWindow", "0"))
        self.push3.setText(_translate("MainWindow", "EUSART"))
        self.push4.setText(_translate("MainWindow", "adafruit io"))
        self.push1.setText(_translate("MainWindow", "Manual"))
        self.push2.setText(_translate("MainWindow", "EEPROM"))
        self.label_4.setText(_translate("MainWindow", "Base:"))
        self.label_5.setText(_translate("MainWindow", "Brazo:"))
        self.label_6.setText(_translate("MainWindow", "Antebrazo:"))
        self.label_7.setText(_translate("MainWindow", "Garra:"))
        self.label_8.setText(_translate("MainWindow", "<html><head/><body><p><span style=\" font-size:9pt; font-weight:600;\">Control de Brazo Mecánico</span></p></body></html>"))


    # Definicion de funciones aun dentro de Qt designer
    def push_Manual (self):
        if self.push1.isChecked():
            print('Modo Manual')
            ser.open()
            ser.write(bytes(str('@'), 'utf-8'))         # Escribir el valor del modo manual al puerto serial
            ser.close()
            print(bytes(str('@'), 'utf-8'))
            self.push2.setEnabled(False)                # Desactiva los otros botones
            self.push3.setEnabled(False)
            self.push4.setEnabled(False)

        else:   
            self.push2.setEnabled(True)                 # Activa los otros botones
            self.push3.setEnabled(True)
            self.push4.setEnabled(True)
            ser.open()
            ser.write(bytes(str('<'), 'utf-8'))         # Escribir el valor del modo 0 al puerto serial
            ser.close()
            
    def push_EEPROM (self):
        if self.push2.isChecked():
            print('Modo EEPROM')
            ser.open()
            ser.write(bytes(str('?'), 'utf-8'))         # Escribir el valor del modo EPPROM al puerto serial
            ser.close()
            print(bytes(str('?'), 'utf-8'))
            self.push1.setEnabled(False)
            self.push3.setEnabled(False)
            self.push4.setEnabled(False)
        else: 
            self.push1.setEnabled(True)
            self.push3.setEnabled(True)
            self.push4.setEnabled(True)
            ser.open()
            ser.write(bytes(str('<'), 'utf-8'))         # Escribir el valor del modo 0 al puerto serial
            ser.close()
        
    def push_EUSART (self):
        if self.push3.isChecked():
            print('Modo EUSART')
            ser.open()
            ser.write(bytes(str('>'), 'utf-8'))         # Escribir el valor del modo EUSART al puerto serial
            ser.close()
            print(bytes(str('>'), 'utf-8'))
            self.push1.setEnabled(False)
            self.push2.setEnabled(False)
            self.push4.setEnabled(False)
            
            self.slider1.setEnabled(True)               # Activa el control EUSART con sliders y un boton
            self.slider2.setEnabled(True)
            self.slider3.setEnabled(True)
            self.push5.setEnabled(True)
            
        else: 
            self.push1.setEnabled(True)                     
            self.push2.setEnabled(True)
            self.push4.setEnabled(True)
            
            self.slider1.setEnabled(False)              # Desactiva sliders y boton
            self.slider2.setEnabled(False)
            self.slider3.setEnabled(False)
            self.push5.setEnabled(False)
            ser.open()
            ser.write(bytes(str('<'), 'utf-8'))         # Escribir el valor del modo 0 al puerto serial
            ser.close()
            
    def push_adafruit_io (self):
        print('Modo adafruit io')
        ser.open()
        ser.write(bytes(str('='), 'utf-8'))             # Escribir el valor del modo Adafruit_IO al puerto serial
        ser.close()
        print(bytes(str('='), 'utf-8'))
        receive_data();                                 # Enviar datos
        ser.open()
        ser.write(bytes(str('<'), 'utf-8'))             # Escribir el valor del modo 0 al puerto serial
        ser.close()
            
    def push_Garra (self):
        # print(self.slider1.value())
        # print(self.slider2.value())
        # print(self.slider3.value())
        if self.push5.isChecked():
            self.push5.setText("Cerrada")
            # print(1)
        else:
            self.push5.setText("Abierta")
            # print(0)
        # send_eusart();
            
    def slider1_fun (self):
        self.label1.setText(str(self.slider1.value()))
        # send_eusart()
        
    def slider2_fun (self):
        self.label2.setText(str(self.slider2.value()))
        # send_eusart()
    
    def slider3_fun (self):
        self.label3.setText(str(self.slider3.value()))
        # send_eusart();
        
    #def send_ser (self):
        # print(self.slider1.value())
        # print(self.slider2.value())
        # print(self.slider3.value())
        # if self.push5.isChecked():
            # print(1)
       # else:
           # print(0)
            
    def send_eusart (self):         # Funcion para enviar los datos del modo EUSART

        data_base = int(self.slider1.value())  # Recibir datos de slider base
        send_data(data_base)    # Enviar
        print(data_base)
        time.sleep(0.5)
        
        data_brazo = int(self.slider2.value())  # Recibir datos de slider brazo
        send_data(data_brazo)
        print(data_brazo)
        time.sleep(0.5)
        
        data_antebrazo = int(self.slider3.value())  # Recibir datos de slider antebrazo
        send_data(data_antebrazo)
        print(data_antebrazo)
        time.sleep(0.5)
        
        if self.push5.isChecked():
            data_garra = 1
        else:
            data_garra = 0
        
        ser.open()
        ser.write(bytes(str(data_garra), 'utf-8'))
        ser.close()
        print(data_garra)
        time.sleep(0.5)
        
if __name__ == "__main__":
    import sys
    app = QtWidgets.QApplication(sys.argv)
    MainWindow = QtWidgets.QMainWindow()
    ui = Ui_MainWindow()
    ui.setupUi(MainWindow)
    MainWindow.show()
    sys.exit(app.exec_())