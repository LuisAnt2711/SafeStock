import cv2
import easyocr
from flask import Flask, Response

app = Flask(__name__)

print("A carregar o leitor de números, aguarde um instante...")
reader = easyocr.Reader(['en'], gpu=True)

# Tenta abrir a webcam principal ou secundária
cap = cv2.VideoCapture(0)
if not cap.isOpened():
    cap = cv2.VideoCapture(1)

numeros_alvo = {'1', '2', '3', '4', '5', '6'}


def gerar_frames():
    while True:
        success, frame = cap.read()
        if not success:
            continue

        resultados = reader.readtext(frame)
        for (bbox, texto, confianca) in resultados:
            texto_limpo = "".join(filter(str.isdigit, texto))
            if confianca > 0.4 and texto_limpo in numeros_alvo:
                (ptA, ptB, ptC, ptD) = bbox
                ptA = (int(ptA[0]), int(ptA[1]))
                ptC = (int(ptC[0]), int(ptC[1]))
                cv2.rectangle(frame, ptA, ptC, (0, 255, 0), 2)
                cv2.putText(frame, f"numero {texto_limpo}", (ptA[0], ptA[1] - 10),
                            cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

        _, buffer = cv2.imencode('.jpeg', frame)
        yield (b'--frame\r\n'
               b'Content-Type: image/jpeg\r\n\r\n' + buffer.tobytes() + b'\r\n')


@app.route('/video_feed')
def video_feed():
    return Response(gerar_frames(), mimetype='multipart/x-mixed-replace; boundary=frame')


if __name__ == '__main__':
    print("Servidor de vídeo iniciado em http://127.0.0.1:5000/video_feed")
    app.run(host='0.0.0.0', port=5000, debug=False, threaded=True)