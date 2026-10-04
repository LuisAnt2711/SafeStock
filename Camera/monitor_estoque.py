"""
Monitor de estoque com marcadores ArUco.

Cada fila da prateleira tem 4 marcadores colados no piso, um sob cada caixa.
Caixa no lugar = marcador coberto. Caixa retirada = marcador visível.
Estoque da fila = 4 - (marcadores visíveis daquela fila).

Requisitos:  pip install opencv-python   (versão 4.7 ou mais nova)
Uso:         python monitor_estoque.py     (tecla q para sair)
"""

import csv
import time
import unicodedata
from datetime import datetime

import cv2

# ---------------------------------------------------------------- configuração
CAMERA = 1                 # 0 = primeira câmera; tente 1 para a USB externa
LARGURA, ALTURA = 1920, 1080
CAIXAS_POR_FILA = 4
FRAMES_ESTAVEIS = 20       # frames seguidos para aceitar uma mudança (filtra a mão)
ARQUIVO_LOG = 'estoque_log.csv'
DICIONARIO = cv2.aruco.DICT_4X4_100

# Filas na ordem da prateleira: de cima para baixo, da esquerda para a direita.
# O marcador de ID = fila * 4 + (posição - 1), com posição 1 = frente.
FILAS = [
    'Remédio A', 'Remédio B', 'Remédio C', 'Remédio D', 'Remédio E',   # prateleira 1
    'Remédio F', 'Remédio G', 'Remédio H', 'Remédio J', 'Remédio K',   # prateleira 2
    'Remédio L', 'Remédio M', 'Remédio N', 'Remédio P', 'Remédio Q',   # prateleira 3
]
# -----------------------------------------------------------------------------

TOTAL_IDS = len(FILAS) * CAIXAS_POR_FILA


def fila_e_posicao(marcador_id):
    """Converte o ID do marcador em (índice da fila, posição 1..4)."""
    return marcador_id // CAIXAS_POR_FILA, marcador_id % CAIXAS_POR_FILA + 1


def sem_acento(texto):
    """O cv2.putText não desenha acentos; remove para exibir na tela."""
    return unicodedata.normalize('NFKD', texto).encode('ascii', 'ignore').decode()


class Monitor:
    """Guarda o estado estável de cada marcador e gera eventos de mudança."""

    def __init__(self):
        self.visivel = {i: False for i in range(TOTAL_IDS)}   # estado aceito
        self.contagem = {i: 0 for i in range(TOTAL_IDS)}      # frames divergentes
        self.calibrado = False
        self.frames = 0

    def atualizar(self, ids_no_frame):
        """Recebe os IDs vistos no frame atual e devolve a lista de eventos."""
        eventos = []
        self.frames += 1
        for i in range(TOTAL_IDS):
            visto = i in ids_no_frame
            if visto == self.visivel[i]:
                self.contagem[i] = 0
                continue
            self.contagem[i] += 1
            if self.contagem[i] >= FRAMES_ESTAVEIS:
                self.visivel[i] = visto
                self.contagem[i] = 0
                if self.calibrado:
                    fila, pos = fila_e_posicao(i)
                    tipo = 'RETIRADA' if visto else 'REPOSICAO'
                    eventos.append((tipo, fila, pos, i))
        # Os primeiros frames servem só para ler o estado inicial, sem eventos.
        if not self.calibrado and self.frames > FRAMES_ESTAVEIS + 5:
            self.calibrado = True
        return eventos

    def estoque(self, fila):
        inicio = fila * CAIXAS_POR_FILA
        vazios = sum(self.visivel[inicio + p] for p in range(CAIXAS_POR_FILA))
        return CAIXAS_POR_FILA - vazios

    def fila_inconsistente(self, fila):
        """Verdadeiro se há caixa na frente de uma posição vazia (ordem estranha)."""
        inicio = fila * CAIXAS_POR_FILA
        estado = [self.visivel[inicio + p] for p in range(CAIXAS_POR_FILA)]
        # O esperado é vazio na frente e ocupado atrás: True, True, False, False
        return estado != sorted(estado, reverse=True)


def desenhar_painel(frame, monitor):
    y = 30
    for fila, nome in enumerate(FILAS):
        qtd = monitor.estoque(fila)
        cor = (0, 200, 0) if qtd > 1 else (0, 165, 255) if qtd == 1 else (0, 0, 255)
        aviso = ' ?' if monitor.fila_inconsistente(fila) else ''
        texto = f'{fila + 1:02d} {sem_acento(nome)}: {qtd}{aviso}'
        cv2.putText(frame, texto, (10, y), cv2.FONT_HERSHEY_SIMPLEX, 0.6, (0, 0, 0), 4)
        cv2.putText(frame, texto, (10, y), cv2.FONT_HERSHEY_SIMPLEX, 0.6, cor, 2)
        y += 24


def main():
    detector = cv2.aruco.ArucoDetector(cv2.aruco.getPredefinedDictionary(DICIONARIO))
    cap = cv2.VideoCapture(CAMERA)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, LARGURA)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, ALTURA)
    if not cap.isOpened():
        raise SystemExit('Não consegui abrir a câmera. Confira o índice em CAMERA.')

    monitor = Monitor()
    avisou_inicio = False

    with open(ARQUIVO_LOG, 'a', newline='', encoding='utf-8') as arquivo:
        log = csv.writer(arquivo)
        if arquivo.tell() == 0:
            log.writerow(['data_hora', 'evento', 'fila', 'produto', 'posicao', 'estoque_fila'])

        while True:
            ok, frame = cap.read()
            if not ok:
                time.sleep(0.1)
                continue

            cinza = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
            cantos, ids, _ = detector.detectMarkers(cinza)
            vistos = set(ids.flatten().tolist()) if ids is not None else set()
            vistos = {i for i in vistos if i < TOTAL_IDS}

            for tipo, fila, pos, _id in monitor.atualizar(vistos):
                agora = datetime.now().strftime('%Y-%m-%d %H:%M:%S')
                qtd = monitor.estoque(fila)
                print(f'{agora}  {tipo:9s}  fila {fila + 1:02d}  {FILAS[fila]}  '
                      f'posição {pos}  ->  restam {qtd}')
                log.writerow([agora, tipo, fila + 1, FILAS[fila], pos, qtd])
                arquivo.flush()

            if monitor.calibrado and not avisou_inicio:
                avisou_inicio = True
                print('Estoque inicial:')
                for fila, nome in enumerate(FILAS):
                    print(f'  fila {fila + 1:02d}  {nome}: {monitor.estoque(fila)}')

            cv2.aruco.drawDetectedMarkers(frame, cantos, ids)
            desenhar_painel(frame, monitor)
            cv2.imshow('Monitor de estoque (q para sair)', frame)
            if cv2.waitKey(1) & 0xFF == ord('q'):
                break

    cap.release()
    cv2.destroyAllWindows()


if __name__ == '__main__':
    main()
