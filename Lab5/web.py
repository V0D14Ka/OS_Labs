from flask import Flask, render_template, request
import requests
import matplotlib.pyplot as plt
from io import BytesIO
import base64

app = Flask(__name__)

# Адрес сервера с температурными данными
SERVER_URL = "http://localhost:8080"

@app.route('/', methods=['GET', 'POST'])
def index():
    # Получаем дату и время начала и окончания из формы
    start_datetime = request.form.get('start_datetime', '1970-01-01T00:00:00')  # Дата и время начала
    end_datetime = request.form.get('end_datetime', '2100-01-01T00:00:00')      # Дата и время окончания
    print(start_datetime)
    print(end_datetime)
    try:
        # Запрос к серверу для получения последней температуры и времени
        last_temperature_response = requests.get(f"{SERVER_URL}/temperature")
        last_temperature = None
        last_timestamp = None
        if last_temperature_response.status_code == 200:
            last_temperature_data = last_temperature_response.json()
            last_temperature = last_temperature_data.get('temperature')
            last_timestamp = last_temperature_data.get('timestamp')
        else:
            last_temperature = "Не удалось получить последнюю температуру"
            last_timestamp = "Не удалось получить время"

        # Запрос к серверу для получения истории температур с указанным периодом
        params = {'start_datetime': start_datetime, 'end_datetime': end_datetime}
        response = requests.get(f"{SERVER_URL}/history", params=params)
        
        if response.status_code == 200:
            data = response.json()

            # Подготовка данных для графика
            timestamps = [entry['timestamp'] for entry in data]
            temperatures = [entry['temperature'] for entry in data]

            # Создание графика
            plt.figure(figsize=(10, 5))
            plt.plot(timestamps, temperatures, marker='o', linestyle='-', color='b')
            plt.xticks(rotation=45)
            plt.xlabel('Время')
            plt.ylabel('Температура')
            plt.title(f'Температура с {start_datetime} по {end_datetime}')

            # Сохранение графика в изображение
            img = BytesIO()
            plt.tight_layout()
            plt.savefig(img, format='png')
            img.seek(0)
            graph_url = base64.b64encode(img.getvalue()).decode('utf-8')

            return render_template('index.html', data=data, graph_url=graph_url, start_datetime=start_datetime, end_datetime=end_datetime, 
                                   last_temperature=last_temperature, last_timestamp=last_timestamp)
        else:
            return "Не удалось получить данные с сервера", 500
    except requests.exceptions.RequestException as e:
        return str(e), 500


if __name__ == '__main__':
    app.run(debug=True)
