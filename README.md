# ESP32_Smartphone_as_Remote_Control
Using a ESP32 as WiFi Acces Point to control a ROLLER with a smartphone
 En este post voy a explicar como he modificado un roller por control remoto para volverlo mas sensible al mando.



El roller que he modificado es el coche teledirigido ZWN 1:12 / 1:16 4WD 86A que se puede encontrar en este link. Aunque se puede usar cualquier juguete similar.





Las ruedas de este Roller tienen un comportamiento muy brusco al tener solo dos "modos": encendido o apagado, (velocidad maxima o parado). Esto hace que sea muy dificil de manejar. Tal como se ve en este video:







Para que el roler sea mas facil de controlar hay que cambiar la electrónica del roller para que los motores de sus 4 ruedas tengan un comportamiento mas suave. Es decir conseguir que sus ruedas puedan girar a cualquier velocidad entre la máxima y cero.



     PRIMERA PARTE    
     MOTORES    
Para conseguir este comportamiento "analogico" de los motores, he utilizado la tecnica PWM (Pulse Width Modulated) ...ancho de pulso modulado, en la que se alimentan los motores D.C. con cortos pulsos del orden de los milisegundos (0,2 ms en este caso) , muy seguidos. De esta manera, controlando la duracion de los pulsos, se puede controlar la velocidad de giro del motor.





Este pulso es generado desde la placa de desarrollo ESP32 y se amplifica con el módulo L298N Driver Motor





Este modulo puede proporcionar la potencia necesaria para accionar pequeños motores DC en diferentes configuraciones, (motores DC en un solo sentido, motores DC en dos sentidos, motores paso, a paso...) en mi caso. necesito accionar un motor para cada rueda del ROLLER. O sea 4 motores.





Cada uno de estos modulos activan sus 4 salidas en funcion de la tension que se le aplique en sus entradas. De esta manera podemos controlar la polaridad que recibe cada uno de los motores, y por lo tanto, su sentido de giro.




En lugar de enviar corriente continua, el Driver L298N puede controlar la velocidad de giro enviando al motor una señal PWM a traves del pin ENABLE, diseñado para este fin.

El Driver L298N puede de esta manera, usar las señales que envíe desde el ESP32 para dar la potencia suficiente para que gire el motor. Pero para que esto funcione, debemos alimentarlo adecuadamente. Mediante su entrada V+12 elevamos la tension de las salidas GPIO del ESP32 a la tension máxima de 12V. 
Este Driver tiene la ventaja de proporcionar una salida de 5V diseñada para alimentar nuestras placas de desarrollo, como la ESP32, ahorrandonos así tener que usar modulos de alimentacion adicionales. Sencillamente conectaremos la tension de las baterías a la entrada V+12 y GND del L298N y nuestra placa de desarrollo ESP32 a los pines GND y +5V.

El circuito necesita 3 vias de comunicacion para cada uno de los motores. Por lo tanto, el ESP32 necesita 12 pines para controlar los dos L298N. Cuatro de ellos capaces de manejar señales PWM. Así la conexion de datos entre modulos queda de esta manera:




Como control remoto he utilizado un telefono. La interfaz de usuario he usado una pagina HTML controlada por Javascript.



El Joystick de la izquierda controla la rotacion, el de la derecha, controla la traslacion. Las tablas centrales tienen algunos datos útiles para la depuracion del programa.



Esta interfaz de usuario envía las coordenadas de los joysticks a la placa ESP32 a través de un archivo JSON muy sencilloque solo incorpora cuatro pares de datos: X1, Y1, X2 y Y2. Estos datos son traducidos por el ESP32 en las señales que deben recibir los motores para girar en el sentido adecuado.

Para avanzar... las cuatro ruedas deben girar hacia adelante.
Para retroceder... las cuatro ruedas deben girar hacia atras.





Para hacer una rotacion sobre su propio eje, si es a la izquierda (contra sentido de reloj) las dos ruedas de la derecha deben girar hacia adelante y las de la izquierda deben girar hacia atras... si la rotacion es la contraria, las ruedas tambien giraran al contrario.
Para hacer traslaciones de izquierda a derecha, depende de la disposicion de las ruedas. En este modelo de Roller. para trasladarlo a la izquierda, las ruedas Right-Front y  Left-Back deben girar hacia adelante y las otras dos hacia atras. Si la traslacion es la contraria, los giros de las ruedas también lo serán.



He elegido el color Verde de las flechas como signo positivo y el rojo como signo negativo. tanto en las coordenadas de los joysticks como en el sentido de giro de los motores.
Cuando el Joystick y la rueda coincida lo he llamado "relacion Directa" y cuando no coinciden "relacion Inversa. De esta manrea, en relacion Directa, el signo de la coordenada del Joystick es el mismo que el signo de rotacion del motor.




De esta manera, por ejemplo el calculo de potencia del motor Left-Front viene dado por la suma de la componente vertical del joystick izquierdo... menos la componente horizontal del mismo joystick... mas la componente vertical del joystick derecho... menos la componente horizontal del mismo joystick.




Para aprovechar al maximo la relacion de joystick- motores, el valor maximo que puede alcanzar cualquiera de los joysticks es el mismo que el valor máximo de la potencia del motor. Y, si al sumarse dos joysticks se sobrepasa el máximo del motor, se recortam el valor. 

Por ejemplo, el cálculo de la potencia del motor Front-Left queda así:


float Motor_LF_Update ()
{
    float DesiredSpeed=ESP32_Y-ESP32_X+ESP32_Y2-ESP32_X2;

    if(DesiredSpeed>ESP32_MAX)DesiredSpeed=ESP32_MAX;
    if(DesiredSpeed<-ESP32_MAX)DesiredSpeed=-ESP32_MAX;

    float Speed= 255 * (DesiredSpeed / ESP32_MAX);

ledcWrite(L298N_ENA_LED_CHANEL, abs(Speed)); 
 
if(Speed==0) 
{ 
digitalWrite(L298N_IN1, 0);
digitalWrite(L298N_IN2, 0); 
}
 
if(Speed<0) 
{ 
digitalWrite(L298N_IN1, 0); 
digitalWrite(L298N_IN2, 1); 
 }
 
if(Speed>0) 
{ 
    digitalWrite(L298N_IN1, 1); 
    digitalWrite(L298N_IN2, 0); 
}
return Speed;
}

Observa que a la entrada L298N_ENA_LED_CHANEL representa el pin que controla la entrada ENABLE_A del driver... por donde se transmite la señal PWM. Y las entradas L298N_IN1 y L298N_IN2. representan las lineas que transmiten la direccion de giro.

Puedes descargar el programa completo en este enlace de github.

     SEGUNDA PARTE    
     TRANSMISION     

Para que el control remoto funcione correctamente, el ROLLER y el SMARTPHONE deben estar en comunicacion constante.

El ESP32 dispone de varias formas de comunicarse con el exterior, Wifi, Bluetooth, ESPNow, y si pensamos en modulos de expansion, las posibilidades se vuelven infinitas.

En este caso he escogido utliizar transmision por WiFi... configurando el ESP32 como acces point, para no tener que depender de un router.

Las ventajas de este método de transmision son varias:
Un gran numero de dispositivos pueden usarse como transmisor. Moviles, tablets, incluso un laptop o PC se podría llegar a usar.
La transmisión wifi permite que la interfaz sea una pagina HTML. Al poder leerse desde un navegador, no se requiere instalar software adicional.
Orientando el sistema a la transmision por WiFi facilita su futura adaptacion al control remoto por Internet.
Al usar las capacidades del ESP32 de convertirse en router, eliminamos la principal pega de usar Wifi: dependencia de router externo.
La forma de convertir el ESP32 en un router y de cargar el HTML en su memoria flash quedan fuera del ambito de este blog. Me concentraré en explicar las funciones y metodos de comunicaciones del ESP32 y de JavaScrip/HTML.

Ademas de los datos que se transmiten del smartphone al ROLLER, he configurado las transmisiones para que el ROLLER también pueda transmitir informacion al smartphone. En este caso, transmitirá  la potencia palculada para cada motor.

Un resumen podría ser este:



La comunicacion empieza por el lado HTML, desde el smartfone... se crea una cadena Json con los datos referentes a la posicion de los joysticks.(en el ejemplo solo he puedto un joystick para simplificar).

let param = JSON.stringify({ "ESP32_X": ESP32_X, "ESP32_Y": ESP32_Y });
%3CmxGraphModel%3E%3Croot%3E%3CmxCell%20id%3D%220%22%2F%3E%3CmxCell%20id%3D%221%22%20parent%3D%220%22%2F%3E%3CmxCell%20id%3D%222%22%20value%3D%22%22%20style%3D%22rounded%3D1%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3BarcSize%3D7%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22-3%22%20y%3D%2214%22%20width%3D%22483%22%20height%3D%22516%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%223%22%20value%3D%22%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(121%20%2C%2094%20%2C%2038)%20%3B%20font-family%3A%20%26amp%3Bquot%3Bconsolas%26amp%3Bquot%3B%20%2C%20%26amp%3Bquot%3Bcourier%20new%26amp%3Bquot%3B%20%2C%20monospace%20%3B%20font-size%3A%2014px%20%3B%20background-color%3A%20rgb(255%20%2C%20255%20%2C%20255)%26quot%3B%26gt%3BrequestByJson()%26lt%3B%2Fspan%26gt%3B%22%20style%3D%22rounded%3D0%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%2220%22%20width%3D%22150%22%20height%3D%2230%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%224%22%20value%3D%22%22%20style%3D%22outlineConnect%3D0%3Bdashed%3D0%3BverticalLabelPosition%3Dbottom%3BverticalAlign%3Dtop%3Balign%3Dcenter%3Bhtml%3D1%3Bshape%3Dmxgraph.aws3.automation%3BfillColor%3D%23759C3E%3BgradientColor%3Dnone%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22170%22%20y%3D%22-5.5%22%20width%3D%2239.48%22%20height%3D%2241%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%225%22%20value%3D%22%26lt%3Bdiv%20style%3D%26quot%3Bbackground-color%3A%20rgb(255%20%2C%20255%20%2C%20255)%20%3B%20font-family%3A%20%26amp%3B%2334%3Bconsolas%26amp%3B%2334%3B%20%2C%20%26amp%3B%2334%3Bcourier%20new%26amp%3B%2334%3B%20%2C%20monospace%20%3B%20font-size%3A%2014px%20%3B%20line-height%3A%2019px%26quot%3B%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%230000ff%26quot%3B%26gt%3Blet%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3Bparam%26lt%3B%2Fspan%26gt%3B%20%3D%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3BJSON%26lt%3B%2Fspan%26gt%3B.%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23795e26%26quot%3B%26gt%3Bstringify%26lt%3B%2Fspan%26gt%3B(%7B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23a31515%26quot%3B%26gt%3B%26quot%3BESP32_X%26quot%3B%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3BESP32_X%26lt%3B%2Fspan%26gt%3B%2C%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23a31515%26quot%3B%26gt%3B%26quot%3BESP32_Y%26quot%3B%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3BESP32_Y%26lt%3B%2Fspan%26gt%3B%20%7D)%3B%26lt%3B%2Fdiv%26gt%3B%22%20style%3D%22rounded%3D0%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%2220%22%20y%3D%2250%22%20width%3D%22430%22%20height%3D%2260%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%226%22%20value%3D%22%26lt%3Bdiv%20style%3D%26quot%3Bbackground-color%3A%20rgb(255%20%2C%20255%20%2C%20255)%20%3B%20font-family%3A%20%26amp%3B%2334%3Bconsolas%26amp%3B%2334%3B%20%2C%20%26amp%3B%2334%3Bcourier%20new%26amp%3B%2334%3B%20%2C%20monospace%20%3B%20font-size%3A%2014px%20%3B%20line-height%3A%2019px%26quot%3B%26gt%3B%26lt%3Bdiv%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%230000ff%26quot%3B%26gt%3Blet%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3Bresponse%26lt%3B%2Fspan%26gt%3B%20%3D%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23af00db%26quot%3B%26gt%3Bawait%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23795e26%26quot%3B%26gt%3Bfetch%26lt%3B%2Fspan%26gt%3B(%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23a31515%26quot%3B%26gt%3B%26quot%3B%2F%26quot%3B%26lt%3B%2Fspan%26gt%3B%2C%20%7B%26lt%3B%2Fdiv%26gt%3B%26lt%3Bdiv%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3Bmethod%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(163%20%2C%2021%20%2C%2021)%26quot%3B%26gt%3B'POST'%26lt%3B%2Fspan%26gt%3B%2C%26lt%3B%2Fdiv%26gt%3B%26lt%3Bdiv%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3Bheaders%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%7B%26lt%3B%2Fdiv%26gt%3B%26lt%3Bdiv%26gt%3B'%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(163%20%2C%2021%20%2C%2021)%26quot%3B%26gt%3BAccept'%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(163%20%2C%2021%20%2C%2021)%26quot%3B%26gt%3B'application%2Fjson'%2C%26lt%3B%2Fspan%26gt%3B%26lt%3B%2Fdiv%26gt%3B%26lt%3Bdiv%26gt%3B'%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(163%20%2C%2021%20%2C%2021)%26quot%3B%26gt%3BContent-type'%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(163%20%2C%2021%20%2C%2021)%26quot%3B%26gt%3B'application%2Fjson'%26lt%3B%2Fspan%26gt%3B%7D%2C%26lt%3B%2Fdiv%26gt%3B%26lt%3Bdiv%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3Bbody%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3B%3A%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20rgb(0%20%2C%2016%20%2C%20128)%26quot%3B%26gt%3Bparam%26lt%3B%2Fspan%26gt%3B%7D)%3B%26lt%3B%2Fdiv%26gt%3B%26lt%3B%2Fdiv%26gt%3B%22%20style%3D%22rounded%3D0%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%2220%22%20y%3D%22135%22%20width%3D%22430%22%20height%3D%22145%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%227%22%20style%3D%22edgeStyle%3DorthogonalEdgeStyle%3Bcurved%3D1%3Brounded%3D0%3BorthogonalLoop%3D1%3BjettySize%3Dauto%3Bhtml%3D1%3BentryX%3D0.535%3BentryY%3D0.655%3BentryDx%3D0%3BentryDy%3D0%3BentryPerimeter%3D0%3BstrokeWidth%3D2%3BfontSize%3D20%3B%22%20edge%3D%221%22%20source%3D%228%22%20target%3D%226%22%20parent%3D%221%22%3E%3CmxGeometry%20relative%3D%221%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%228%22%20value%3D%22%22%20style%3D%22shape%3DcurlyBracket%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3Brotation%3D-90%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%2283%22%20y%3D%2271%22%20width%3D%2230%22%20height%3D%2246%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%229%22%20value%3D%22%22%20style%3D%22shape%3DcurlyBracket%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3Brotation%3D90%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22235%22%20y%3D%22220%22%20width%3D%2230%22%20height%3D%2250%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2210%22%20value%3D%22%26lt%3Bdiv%20style%3D%26quot%3Bbackground-color%3A%20rgb(255%20%2C%20255%20%2C%20255)%20%3B%20font-family%3A%20%26amp%3B%2334%3Bconsolas%26amp%3B%2334%3B%20%2C%20%26amp%3B%2334%3Bcourier%20new%26amp%3B%2334%3B%20%2C%20monospace%20%3B%20font-size%3A%2014px%20%3B%20line-height%3A%2019px%26quot%3B%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%230000ff%26quot%3B%26gt%3Blet%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3Bcontent%26lt%3B%2Fspan%26gt%3B%20%3D%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23af00db%26quot%3B%26gt%3Bawait%26lt%3B%2Fspan%26gt%3B%20%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3Bresponse%26lt%3B%2Fspan%26gt%3B.%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23795e26%26quot%3B%26gt%3Bjson%26lt%3B%2Fspan%26gt%3B()%3B%26lt%3B%2Fdiv%26gt%3B%22%20style%3D%22rounded%3D0%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%2220%22%20y%3D%22310%22%20width%3D%22430%22%20height%3D%2240%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2211%22%20value%3D%22%26%2310%3B%26%2310%3B%26lt%3Bdiv%20style%3D%26quot%3Bcolor%3A%20rgb(0%2C%200%2C%200)%3B%20background-color%3A%20rgb(255%2C%20255%2C%20255)%3B%20font-family%3A%20consolas%2C%20%26amp%3Bquot%3Bcourier%20new%26amp%3Bquot%3B%2C%20monospace%3B%20font-weight%3A%20normal%3B%20font-size%3A%2014px%3B%20line-height%3A%2019px%3B%26quot%3B%26gt%3B%26lt%3Bdiv%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3BLeftValue3%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23000000%26quot%3B%26gt%3B.%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3BinnerHTML%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23000000%26quot%3B%26gt%3B%20%3D%20%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3Bcontent%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23000000%26quot%3B%26gt%3B.%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23001080%26quot%3B%26gt%3Banswer%26lt%3B%2Fspan%26gt%3B%26lt%3Bspan%20style%3D%26quot%3Bcolor%3A%20%23000000%26quot%3B%26gt%3B%3B%26lt%3B%2Fspan%26gt%3B%26lt%3B%2Fdiv%26gt%3B%26lt%3B%2Fdiv%26gt%3B%26%2310%3B%26%2310%3B%22%20style%3D%22rounded%3D0%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%2220%22%20y%3D%22382%22%20width%3D%22430%22%20height%3D%2260%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2212%22%20style%3D%22edgeStyle%3DorthogonalEdgeStyle%3Bcurved%3D1%3Brounded%3D0%3BorthogonalLoop%3D1%3BjettySize%3Dauto%3Bhtml%3D1%3BentryX%3D0.1%3BentryY%3D0.5%3BentryDx%3D0%3BentryDy%3D0%3BentryPerimeter%3D0%3BstrokeWidth%3D2%3BfontSize%3D20%3BexitX%3D0.1%3BexitY%3D0.5%3BexitDx%3D0%3BexitDy%3D0%3BexitPerimeter%3D0%3B%22%20edge%3D%221%22%20source%3D%2213%22%20target%3D%2214%22%20parent%3D%221%22%3E%3CmxGeometry%20relative%3D%221%22%20as%3D%22geometry%22%3E%3CArray%20as%3D%22points%22%3E%3CmxPoint%20x%3D%22167%22%20y%3D%22280%22%2F%3E%3CmxPoint%20x%3D%22285%22%20y%3D%22280%22%2F%3E%3C%2FArray%3E%3C%2FmxGeometry%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2213%22%20value%3D%22%22%20style%3D%22shape%3DcurlyBracket%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3Brotation%3D-90%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22152%22%20y%3D%22138%22%20width%3D%2230%22%20height%3D%2270%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2214%22%20value%3D%22%22%20style%3D%22shape%3DcurlyBracket%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3Brotation%3D90%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22265%22%20y%3D%22280%22%20width%3D%2230%22%20height%3D%2270%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2215%22%20style%3D%22edgeStyle%3DorthogonalEdgeStyle%3Bcurved%3D1%3Brounded%3D0%3BorthogonalLoop%3D1%3BjettySize%3Dauto%3Bhtml%3D1%3BstrokeWidth%3D2%3BfontSize%3D20%3BexitX%3D0.1%3BexitY%3D0.5%3BexitDx%3D0%3BexitDy%3D0%3BexitPerimeter%3D0%3BentryX%3D0.1%3BentryY%3D0.5%3BentryDx%3D0%3BentryDy%3D0%3BentryPerimeter%3D0%3B%22%20edge%3D%221%22%20source%3D%2216%22%20target%3D%2217%22%20parent%3D%221%22%3E%3CmxGeometry%20relative%3D%221%22%20as%3D%22geometry%22%3E%3CArray%20as%3D%22points%22%3E%3CmxPoint%20x%3D%22155%22%20y%3D%22370%22%2F%3E%3CmxPoint%20x%3D%22293%22%20y%3D%22370%22%2F%3E%3C%2FArray%3E%3C%2FmxGeometry%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2216%22%20value%3D%22%22%20style%3D%22shape%3DcurlyBracket%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3Brotation%3D-90%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22140%22%20y%3D%22316.25%22%20width%3D%2230%22%20height%3D%2257.5%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3CmxCell%20id%3D%2217%22%20value%3D%22%22%20style%3D%22shape%3DcurlyBracket%3BwhiteSpace%3Dwrap%3Bhtml%3D1%3Brounded%3D1%3Bshadow%3D1%3Bglass%3D0%3Bsketch%3D0%3BfontSize%3D20%3Balign%3Dcenter%3Brotation%3D90%3B%22%20vertex%3D%221%22%20parent%3D%221%22%3E%3CmxGeometry%20x%3D%22280%22%20y%3D%22370%22%20width%3D%2225%22%20height%3D%2255%22%20as%3D%22geometry%22%2F%3E%3C%2FmxCell%3E%3C%2Froot%3E%3C%2FmxGraphModel%3E

Se envía una promesa, ( una promesa es una clase de javascript que facilita la comunicacion cliente/servidor ) desde el smartphone al ESP32 con este json como paramentro

let response = await fetch("/", {
method: 'POST',
headers: {
'Accept': 'application/json',
'Content-type': 'application/json'},
body: param});


el metodo de transmision es POST... de manera que no se actualiza la URL.

en ese punto, el smartfone ya ha hecho su peticion (request) y se queda a la espera de una respuesta del ESP32:

El ESP32 recibe esta peticion (request) del smartphone a trabes del la funcion callback cargada en el objeto "server" de la biblioteca ESP32WebServer a traves de su método "server.on". Cuyo enunciado es:

    server.on("/", HTTP_POST, [](AsyncWebServerRequest* request) {}, NULL,
        [](AsyncWebServerRequest* request,
 uint8_t* data, size_t len, size_t index, size_t total)

A través del parametro data[] la funcion calback el ESP recibe el archivo json enviado por el smartphone. Este parametro se deserializa ( se convierte los bytes en serie en pares de variables separadas dentro de un objeto hecho a medida del json por la clase DinamicJsonDocument). Y una vez deserializado... se comprueba, uno a uno si el ESP32 ha recibido todos los datos que necesita. En este caso, las coordenadas de los joysticks:

             DynamicJsonDocument json_received_doc(200);
            deserializeJson(json_received_doc, data);

                    ...
            
                 if (json_received_doc.containsKey("ESP32_X"))
                    ESP32_X = json_received_doc["ESP32_X"].as<float>();
                if (json_received_doc.containsKey("ESP32_Y"))
                    ESP32_Y = json_received_doc["ESP32_Y"].as<float>();
                if (json_received_doc.containsKey("ESP32_X2"))
                    ESP32_X2 = json_received_doc["ESP32_X2"].as<float>();
                if (json_received_doc.containsKey("ESP32_Y2"))
                    ESP32_Y2 = json_received_doc["ESP32_Y2"].as<float>();


   Acto seguido, el ESP32 construye la respuesta, que en este caso, consiste en el calculo de velocidades de motores del ciclo anterior, almacenado en las veriables globales SpeedRL. SpeedRF,SpeedFL y SpeedFR. Utilizando el puntero a la peticion recibida (request) para formar la respuesta (response) request->send()

                doc["LF"]=(int)SpeedLF;
                doc["RF"]=(int)SpeedRF;
                doc["LB"]=(int)SpeedLB;
                doc["RB"]=(int)SpeedRB;
                String str_doc;
                serializeJson(doc, str_doc);
                request->send(200, "application/json", str_doc);

Esta respuesta es recibida por la promesa que estaba esperando en el smartphone:

                    let content = await response.json();

y traducida en archivo json para extraer los valores de las velocidades y presentarlos en los <div> correspondientes.

            if (content.hasOwnProperty("LF")) MOTOR_LF.innerHTML = content.LF;
            else {/*MOTOR_LF.innerHTML = "--";*/}
            if (content.hasOwnProperty("RF")) MOTOR_RF.innerHTML = content.RF;
            else {/*MOTOR_RF.innerHTML = "--";*/}
            if (content.hasOwnProperty("LB")) MOTOR_LB.innerHTML = content.LB;
            else {/*MOTOR_LB.innerHTML = "--";*/}
            if (content.hasOwnProperty("RB")) MOTOR_RB.innerHTML = content.RB;
            else {/*MOTOR_RB.innerHTML = "--";*/}
            
LA funcion setInterval(requestByJson, 100); en el script del json se encarga de repetir esta 
llamada cada decima de segundo

Todo el proceso se puede resumir en este esquema:





Puedes descargar el programa completo en este enlace de github.
dffdd
