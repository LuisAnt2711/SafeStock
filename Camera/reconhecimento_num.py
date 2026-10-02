import cv2
import easyocr
import threading
import time
from flask import Flask, Response

app = Flask(__name__)

# =====================================================
# CONFIGURAÇÕES
# =====================================================

# Intervalo entre reconhecimentos
# 0.2 = aproximadamente 5 reconhecimentos por segundo
INTERVALO_OCR = 0.2

# Números que queremos reconhecer
numeros_alvo = {'1', '2', '3', '4', '5', '6'}

# =====================================================
# EASY OCR
# =====================================================

print("A carregar o leitor de números, aguarde um instante...")

reader = easyocr.Reader(['en'], gpu=True)

print("Leitor carregado com GPU.")

# =====================================================
# WEBCAM
# =====================================================


cap = cv2.VideoCapture(1)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, 1920)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, 1080)

if not cap.isOpened():
    cap = cv2.VideoCapture(1)

if not cap.isOpened():
    print("ERRO: nenhuma webcam encontrada.")
    exit()

print("Webcam iniciada.")

# =====================================================
# VARIÁVEIS COMPARTILHADAS
# =====================================================

frame_atual = None
resultados_ocr = []

lock_frame = threading.Lock()
lock_resultados = threading.Lock()


# =====================================================
# CAPTURA DA CÂMERA
# =====================================================

def capturar_camera():

    global frame_atual

    while True:

        success, frame = cap.read()

        if not success:
            continue

        with lock_frame:
            frame_atual = frame.copy()


# =====================================================
# RECONHECIMENTO OCR
# =====================================================

def reconhecer_numeros():

    global resultados_ocr

    ultimo_reconhecimento = 0

    while True:

        agora = time.monotonic()

        # Verifica se já passou o intervalo configurado
        if agora - ultimo_reconhecimento < INTERVALO_OCR:
            time.sleep(0.01)
            continue

        # Obtém uma cópia do frame atual
        with lock_frame:

            if frame_atual is None:
                continue

            frame_para_ocr = frame_atual.copy()

        # Faz o reconhecimento
        resultados = reader.readtext(
            frame_para_ocr,
            allowlist='123456',
            low_text = 0.6,
            link_threshold=0.8,
            add_margin=0.05,
            mag_ratio=2,
        )

        novos_resultados = []

        for (bbox, texto, confianca) in resultados:

            texto_limpo = "".join(
                filter(str.isdigit, texto)
            )

            if (
                confianca > 0.7
                and texto_limpo in numeros_alvo
            ):

                novos_resultados.append(
                    (bbox, texto_limpo, confianca)
                )

        # Atualiza os resultados
        with lock_resultados:
            resultados_ocr = novos_resultados

        ultimo_reconhecimento = agora


# =====================================================
# GERAÇÃO DO VÍDEO
# =====================================================

def gerar_frames():

    while True:

        # Obtém o frame mais recente
        with lock_frame:

            if frame_atual is None:
                continue

            frame = frame_atual.copy()

        # Obtém os resultados mais recentes do OCR
        with lock_resultados:
            resultados = list(resultados_ocr)

        # Desenha os resultados no frame
        for (bbox, texto, confianca) in resultados:

            ptA, ptB, ptC, ptD = bbox

            ptA = (
                int(ptA[0]),
                int(ptA[1])
            )

            ptC = (
                int(ptC[0]),
                int(ptC[1])
            )

            cv2.rectangle(
                frame,
                ptA,
                ptC,
                (0, 255, 0),
                2
            )

            cv2.putText(
                frame,
                f"numero {texto}",
                (ptA[0], ptA[1] - 10),
                cv2.FONT_HERSHEY_SIMPLEX,
                0.7,
                (0, 255, 0),
                2
            )

        # Converte para JPEG
        _, buffer = cv2.imencode(
            '.jpeg',
            frame
        )

        yield (
            b'--frame\r\n'
            b'Content-Type: image/jpeg\r\n\r\n'
            + buffer.tobytes()
            + b'\r\n'
        )


# =====================================================
# FLASK
# =====================================================

@app.route('/video_feed')
def video_feed():

    return Response(
        gerar_frames(),
        mimetype='multipart/x-mixed-replace; boundary=frame'
    )


# =====================================================
# INICIALIZAÇÃO
# =====================================================

if __name__ == '__main__':

    # Thread responsável pela câmera
    thread_camera = threading.Thread(
        target=capturar_camera,
        daemon=True
    )

    # Thread responsável pelo OCR
    thread_ocr = threading.Thread(
        target=reconhecer_numeros,
        daemon=True
    )

    thread_camera.start()
    thread_ocr.start()

    print()
    print("======================================")
    print("Servidor de vídeo iniciado!")
    print("======================================")
    print()
    print("Acesse:")
    print("http://127.0.0.1:5000/video_feed")
    print()
    print(f"OCR: {1 / INTERVALO_OCR:.1f} reconhecimentos/segundo")
    print()

    app.run(
        host='0.0.0.0',
        port=5000,
        debug=False,
        threaded=True
    )