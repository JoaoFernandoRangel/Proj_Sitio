clc

antes_meia = Hora(1:3414);
temp_antes_meia = Temp(1:3414);
dps_meia = Hora(3415:end) + 1; %Soma um dia na tabela
temp_dps_meia = Temp(3415:end);

plot(antes_meia, temp_antes_meia);
grid on
hold on
plot(dps_meia, temp_dps_meia);
%%
plot(Dia_21, Temp_21)
hold on
Dia_22_ = Dia_22(:) + 1;
%
plot(Dia_22_, Temp_22)
grid on