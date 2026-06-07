
Prueba = uiopen('Dirección del archivo .xml con los datos de cada prueba');
camaraant = Prueba.Camara(1);
blandoant = Prueba.Blando(1);

for i=1:length(Prueba.Camara)
    if (Prueba.Camara(i) <= camaraant - 0.15 || Prueba.Camara(i) >= camaraant+0.15)
        Prueba.Camara(i) = camaraant;
    end
    if (Prueba.Blando(i) <= blandoant - 0.15 || Prueba.Blando(i) >= blandoant + 0.15)
        Prueba.Blando(i) = blandoant;
    end
    blandoant = Prueba.Blando(i);
    camaraant = Prueba.Camara(i);
end

figure
subplot(3,1,1)
plot(Prueba.Tiempo/1000, Prueba.Camara, 'LineWidth', 2) 
ylim([-0.6 0.1])
xlabel('Tiempo (s)')
ylabel('Presión (bar)')
title('Presión en la cámara de vacío')
legend('Cámara')
subplot(3,1,2)
plot(Prueba.Tiempo/1000, Prueba.Blando,"r",'LineWidth', 2)
hold on
plot(Prueba.Tiempo/1000, Prueba.Consigna,"g", 'LineWidth', 2)

ylim([-0.6 0.1])
xlabel('Tiempo (s)')
ylabel('Presión (bar)')
title('Presión en la parte blanda')
legend('Parte Blanda','Consigna')
subplot(3,1,3)
plot(Prueba.Tiempo/1000, Prueba.Camara, 'LineWidth', 2)
hold on
plot(Prueba.Tiempo/1000, Prueba.Blando,"r", 'LineWidth', 2)
plot(Prueba.Tiempo/1000, Prueba.Consigna,"g", 'LineWidth', 2)

ylim([-0.6 0.1])
xlabel('Tiempo (s)')
ylabel('Presión (bar)')
title('Conjunto de presiones en parte blanda y cámara de vacío')
legend('Camara', 'Parte Blanda','Consigna')
grid on

figure
plot(Prueba.Tiempo/1000, Prueba.Camara, 'LineWidth', 2)
hold on
plot(Prueba.Tiempo/1000, Prueba.Blando,"r", 'LineWidth', 2)
plot(Prueba.Tiempo/1000, Prueba.Consigna,"g", 'LineWidth', 2)

ylim([-0.6 0.1])
xlabel('Tiempo (s)')
ylabel('Presión (bar)')
title('Conjunto de presiones en parte blanda y cámara de vacío')
legend('Camara', 'Parte Blanda','Consigna')

figure
plot(Prueba.Tiempo/1000, Prueba.Blando,"r", 'LineWidth', 2)
hold on
plot(Prueba.Tiempo/1000, Prueba.Consigna,"g", 'LineWidth', 2)

ylim([-0.6 0.1])
xlabel('Tiempo (s)')
ylabel('Presión (bar)')
title('Presión en la parte blanda')
legend('Parte Blanda','Consigna')

figure
plot(Prueba.Tiempo/1000, Prueba.Camara, 'LineWidth', 2)

ylim([-0.6 0])
xlabel('Tiempo (s)')
ylabel('Presión (bar)')
title('Conjunto de presiones en parte blanda y cámara de vacío')
legend('Camara')