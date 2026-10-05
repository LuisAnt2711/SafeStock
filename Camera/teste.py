import cv2
import easyocr

reader = easyocr.Reader((['en']), gpu=False)
cap = cv2.VideoCapture(0)

while True:
    ok, frame = cap.read()
    if not ok:
        break

    gray = cv2.cvtColor(frame, cv2.COLOR_BGR2GRAY)
    results = reader.readtext(gray, allowlist='0123456789')

    for bbox in results:
        if conf > 0.5:
            p1 = tuple(map(int, bbox[0]))
            p2 = tuple(map(int, bbox[2]))
            cv2.rectangle(frame, p1, p2, (0, 255, 0), 2)
            cv2.putText(frame,texto,(p1[0],p1[1]-10),cv2.FONT_HERSHEY_SIMPLEX,1,(0,255,0),2)
            print(texto,round(conf,2))
    cv2.imshow('OCR',frame)
    if cv2.waitKey(1) & 0xFF == ord('q'):
        break

cap.release()
cv2.destroyAllWindows()


