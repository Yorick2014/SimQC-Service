# Сборка и запуск контейнера
1. docker build -t simqc-api .
2. docker run -p 5555:5555 simqc-api

# Загрузка конфигурации
```
curl -X POST http://localhost:5555/api/v1/config \
     -H "Content-Type: application/json" \
     -d '{"common": {"protocol":2,"laser_type":"AttLaser","modulator_type":"polarization","channel_type":"FOCL","photodetector_type":"SPAD","num_pulses":10,"seed_Alice": 1231,"seed_Bob": 2441},
          "laser": {"central_wavelength":1550e-9,"laser_power_w":1e-3,"pulse_duration":1e-9,"attenuation_db":60,"repeat_rate":300e6},
          "q_channel": {"channel_length": 20.0, "chromatic_dispersion": 17.0, "channel_attenuation": 0.2, "is_att": false, "is_crom_disp": false},
          "photodetector": {"pde": 0.95, "dead_time": 0.0, "dcr": 10.0, "time_slot": 5e-9, "afterpulse_prob": 0.01, "afterpulse_delay": 1e-6}}'

```

# Старт
```
curl -X POST http://localhost:5555/api/v1/start \
     -H "Content-Type: application/json" \
     -d '{"start": true}'
```

# Cтатус
```
curl -X GET http://localhost:5555/api/v1/status
```

# Остановить
```
curl -X POST http://localhost:5555/api/v1/stop
```