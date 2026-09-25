import cv2
import easyocr

print("A carregar o leitor de números, aguarde um instante...")
# Inicializa o leitor do EasyOCR
reader = easyocr.Reader(['en'], gpu=True)

# Abre a webcam USB (porta 0)
cap = cv2.VideoCapture(1, cv2.CAP_MSMF)

if not cap.isOpened():
    print("Aviso: Tentando abrir a porta 0 sem MSMF...")
    cap = cv2.VideoCapture(0)

# Números alvo que queremos detetar
numeros_alvo = {'1', '2', '3', '4', '5', '6'}

print("Câmara iniciada! Aponte um número de 1 a 6. Pressione 'q' para sair.")

while True:
    ret, frame = cap.read()
    if not ret:
        print("Não foi possível acessar a webcam. Verifique a conexão.")
        break

    # Lê os textos do fotograma atual
    resultados = reader.readtext(frame)

    for (bbox, texto, confianca) in resultados:
        # Filtra apenas os dígitos numéricos
        texto_limpo = "".join(filter(str.isdigit, texto))

        # Valida a confiança e se o número está entre 1 e 6
        if confianca > 0.4 and texto_limpo in numeros_alvo:
            (ptA, ptB, ptC, ptD) = bbox
            ptA = (int(ptA[0]), int(ptA[1]))
            ptC = (int(ptC[0]), int(ptC[1]))

            # Desenha uma caixa verde à volta do número
            cv2.rectangle(frame, ptA, ptC, (0, 255, 0), 2)

            # Texto exibido na tela
            mensagem = f"numero {texto_limpo} reconhecido"
            cv2.putText(frame, mensagem, (ptA[0], ptA[1] - 10),
                        cv2.FONT_HERSHEY_SIMPLEX, 0.7, (0, 255, 0), 2)

    # Mostra o vídeo na tela
    cv2.imshow("Reconhecedor de Numeros - PyCharm", frame)

    # Aperte a tecla 'q' no teclado para fechar a janela
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

# Liberta a câmara e fecha as janelas com segurança
if cap.isOpened():
    cap.release()

try:
    cv2.destroyAllWindows()
except Exception:
    pass