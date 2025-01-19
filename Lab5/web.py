from flask import Flask, render_template
import requests
import matplotlib.pyplot as plt
from io import BytesIO
import base64

app = Flask(__name__)

# Адрес сервера с температурными данными
SERVER_URL = "http://localhost:8080/history"

@app.route('/')
def index():
    try:
        # Запрос к серверу для получения истории температур
        response = requests.get(SERVER_URL)
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
            plt.title('Изменение температуры')

            # Сохранение графика в изображение
            img = BytesIO()
            plt.tight_layout()
            plt.savefig(img, format='png')
            img.seek(0)
            graph_url = base64.b64encode(img.getvalue()).decode('utf-8')

            return render_template('index.html', data=data, graph_url=graph_url)
        else:
            return "Не удалось получить данные с сервера", 500
    except requests.exceptions.RequestException as e:
        return str(e), 500

if __name__ == '__main__':
    app.run(debug=True)
