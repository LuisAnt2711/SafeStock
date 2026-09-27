import cv2
from flask import Flask, Response, jsonify
import threading
import time
import os

app = Flask(__name__)

# Tenta abrir a webcam principal (0) ou secundária (1)
cap = cv2.VideoCapture(1)
if not cap.isOpened():
    cap = cv2.VideoCapture(1)

# Variável para armazenar a última retirada detetada e partilhar com o site
ultima_retirada_cache = None
lock = threading.Lock()


def monitorizar_prateleira_periodicamente():
    global ultima_retirada_cache

    # Exemplo simulado de monitorização a cada 10 segundos
    # (Pode adaptar aqui a contagem por pixéis/subtracção de imagem se desejar)
    while True:
        time.sleep(10)  # Aguarda 10 segundos

        # Simula ou calcula a retirada (Exemplo: quando detetar a mudança, define o remédio e a quantidade)
        # Para testar, por exemplo, sempre que houver um gatilho físico ou visual:
        evento_detectado = False  # Mude para True quando a câmara detetar a alteração física

        if evento_detectado:
            with lock:
                # Informa qual o medicamento e quantas unidades foram retiradas
                ultima_retirada_cache = {
                    "remedio": "paracetamol",  # Nome do remédio que está na tabela do site
                    "quantidade": 1  # Quantidade a abater
                }
            print("[WEBCAM] Retirada detetada e pronta para abater no estoque!")


@app.route('/video_feed')
def video_feed():
    def gerar_frames():
        while True:
            success, frame = cap.read()
            if not success:
                continue

            # Desenha texto informativo no vídeo
            cv2.putText(frame, "Prateleira Monitorizada (Ativa)", (20, 40),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

            _, buffer = cv2.imencode('.jpeg', frame)
            yield (b'--frame\r\n'
                   b'Content-Type: image/jpeg\r\n\r\n' + buffer.tobytes() + b'\r\n')

    return Response(gerar_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')


@app.route('/ultima_retirada', methods=['GET'])
def ultima_retirada():
    global ultima_retirada_cache
    with lock:
        # Retorna os dados e limpa a cache para não abater repetidamente
        dados = ultima_retirada_cache
        ultima_retirada_cache = None
        return jsonify(dados if dados else {})


if __name__ == '__main__':
    # Inicia a thread de monitorização em segundo plano
    t = threading.Thread(target=monitorizar_prateleira_periodicamente, daemon=True)
    t.start()

    print("Servidor de vídeo iniciado em http://127.0.0.1:5000/video_feed")
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)