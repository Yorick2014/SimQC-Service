# Сборка и запуск контейнера
1. docker build -t simqc-api .
2. docker run -p 5555:5555 simqc-api

# Загрузка конфигурации
```
curl -X POST http://localhost:5555/api/v1/config \
     -H "Content-Type: application/json" \
     -d '{"common": {"protocol":1,"laser_type":"AttLaser","num_pulses":10,"seed_Alice": 10,"seed_Bob": 21},
          "laser": {"central_wavelength":1550,"laser_power_w":1e-3,"pulse_duration":1e-9,"attenuation_db":60,"repeat_rate":300e6}}'
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