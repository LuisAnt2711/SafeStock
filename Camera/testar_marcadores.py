"""
Teste dos marcadores ArUco fora da prateleira.

Mostre os marcadores impressos para a câmera. O script desenha cada marcador
detectado com o ID, a fila e a posição a que ele corresponde, e o tamanho em
pixels com que está aparecendo na imagem.

Requisitos:  pip install opencv-python   (versão 4.7 ou mais nova)
Uso:         python testar_marcadores.py
Teclas:      q = sair    s = salvar uma foto do que a câmera está vendo
"""

import cv2
import numpy as np

CAMERA = 1                 # 0 = primeira câmera; tente 1 para a USB externa
LARGURA, ALTURA = 1920, 1080
CAIXAS_POR_FILA = 4
TAMANHO_MINIMO = 30        # lado em pixels abaixo do qual a leitura fica instável
DICIONARIO = cv2.aruco.DICT_4X4_100

detector = cv2.aruco.ArucoDetector(cv2.aruco.getPredefinedDictionary(DICIONARIO))
cap = cv2.VideoCapture(CAMERA)
cap.set(cv2.CAP_PROP_FRAME_WIDTH, LARGURA)
cap.set(cv2.CAP_PROP_FRAME_HEIGHT, ALTURA)
if not cap.isOpened():
    raise SystemExit('Não consegui abrir a câmera. Confira o índice em CAMERA.')

print('Resolução da câmera:', int(cap.get(cv2.CAP_PROP_FRAME_WIDTH)), 'x',
      int(cap.get(cv2.CAP_PROP_FRAME_HEIGHT)))

ja_vistos = set()
fotos = 0

while True:
    ok, frame = cap.read()
    if not ok:
        continue

    cinza = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    cantos, ids, _ = detector.detectMarkers(cinza)
    limpo = frame.copy()

    if ids is not None:
        cv2.aruco.drawDetectedMarkers(frame, cantos, ids)
        for quad, marcador_id in zip(cantos, ids.flatten().tolist()):
            pts = quad[0]
            # lado médio do marcador na imagem, em pixels
            lado = np.mean([np.linalg.norm(pts[i] - pts[(i + 1) % 4]) for i in range(4)])
            fila = marcador_id // CAIXAS_POR_FILA + 1
            pos = marcador_id % CAIXAS_POR_FILA + 1
            cor = (0, 200, 0) if lado >= TAMANHO_MINIMO else (0, 0, 255)
            texto = f'ID {marcador_id}  fila {fila} pos {pos}  {lado:.0f}px'
            x, y = int(pts[:, 0].min()), int(pts[:, 1].max()) + 22
            cv2.putText(frame, texto, (x, y), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 4)
            cv2.putText(frame, texto, (x, y), cv2.FONT_HERSHEY_SIMPLEX, 0.6, cor, 2)

            if marcador_id not in ja_vistos:
                ja_vistos.add(marcador_id)
                print(f'Detectado pela primeira vez: ID {marcador_id} '
                      f'(fila {fila}, posição {pos}) com {lado:.0f} px de lado')

    qtd = 0 if ids is None else len(ids)
    resumo = f'Agora: {qtd}   Ja vistos: {len(ja_vistos)} de 60'
    cv2.putText(frame, resumo, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 0, 0), 4)
    cv2.putText(frame, resumo, (10, 30), cv2.FONT_HERSHEY_SIMPLEX, 0.8, (0, 255, 255), 2)

    cv2.imshow('Teste dos marcadores (q = sair, s = salvar foto)', frame)
    tecla = cv2.waitKey(1) & 0xFF
    if tecla == ord('q'):
        break
    if tecla == ord('s'):
        fotos += 1
        cv2.imwrite(f'teste_{fotos:02d}.png', limpo)
        cv2.imwrite(f'teste_{fotos:02d}_marcado.png', frame)
        print(f'Fotos salvas: teste_{fotos:02d}.png e teste_{fotos:02d}_marcado.png')

cap.release()
cv2.destroyAllWindows()

faltando = sorted(set(range(60)) - ja_vistos)
print(f'\nMarcadores vistos: {len(ja_vistos)} de 60')
if faltando:
    print('Nunca detectados:', faltando)
