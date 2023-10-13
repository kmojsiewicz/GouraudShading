#include <QApplication>
#include <QLabel>
#include <QPainter>
#include <QTimer>
#include <QtMath>
#include "QDebug"

#define WND_WIDTH   800
#define WND_HEIGHT  600

template <class T>
void swap_data(T& x, T& y)
{
    T temp;
    temp = x;
    x = y;
    y = temp;
}

struct TVertexCol {
    float x;
    float y;
    unsigned char RGB[3];
};

inline void putPixel(int x, int y, int R, int G, int B, QPainter *painter)
{
    if (R < 0) R = 0;
    if (R > 255) R = 255;
    if (G < 0) G = 0;
    if (G > 255) G = 255;
    if (B < 0) B = 0;
    if (B > 255) B = 255;
    QColor Color = QColor(R, G, B, 255);
    painter->setPen(Color);
    painter->drawPoint(x, y);
}


#define SUB_PIX(a) (ceil(a)-a)

void drawTriangle(TVertexCol V1, TVertexCol V2, TVertexCol V3, QPainter *painter)
{
    if (V1.y > V2.y) swap_data(V1, V2);                                 // sort the vertices (V1,V2,V3) by their Y values
    if (V1.y > V3.y) swap_data(V1, V3);
    if (V2.y > V3.y) swap_data(V2, V3);

    if ((int)V1.y == (int)V3.y) return;                                 // check if we have more than a zero height triangle

    /*QColor Col = QColor(255, 255, 255, 255);
    painter->setPen(Col);
    painter->drawLine(V1.x, V1.y, V2.x, V2.y);
    painter->drawLine(V3.x, V3.y, V2.x, V2.y);
    painter->drawLine(V3.x, V3.y, V1.x, V1.y);*/

    // We have to decide whether V2 is on the left side or the right one. We could do that by findng the V4, and
    // check the disatnce from V4 to V2 (V4.y = V2.y). V4 is one the edge (V1V3)
    // Using formula : I(t) = A + t(B-A) we find y = V1.y + t(V3.y - V1.y) and y = V2.y
    // V2.y - V1.y = t(V3.y - V1.y) -->  t = (V2.y - V1.y)/(V3.y - V1.y)
    // V4.x = V1.x + t(V3.x - V1.x) and V4.y = V2.y
    // float distance = V4.x - V2.x
    // if (distance > 0) then the middle vertex is on the left side (V1V3 is the longest edge on the right side)

    float dY21 = 1.0 / ceil(V2.y - V1.y);
    float dY31 = 1.0 / ceil(V3.y - V1.y);
    float dY32 = 1.0 / ceil(V3.y - V2.y);

    float dXdY21 = (float)(V2.x - V1.x) * dY21;                         // dXdY means deltaX/deltaY
    float dXdY31 = (float)(V3.x - V1.x) * dY31;
    float dXdY31tmp = dXdY31;
    float dXdY32 = (float)(V3.x - V2.x) * dY32;

    float dXCol = 1.0 / ((V3.x - V1.x)*ceil(V2.y - V1.y) + (V1.x - V2.x)*ceil(V3.y - V1.y));

    float dRdY21 = (float)(V2.RGB[0] - V1.RGB[0]) * dY21;
    float dRdY31 = (float)(V3.RGB[0] - V1.RGB[0]) * dY31;
    float dRdY31tmp = dRdY31;
    float dRdY32 = (float)(V3.RGB[0] - V2.RGB[0]) * dY32;
    float dRdX   = (float)((V3.RGB[0] - V1.RGB[0])*ceil(V2.y - V1.y) + (V1.RGB[0] - V2.RGB[0])*ceil(V3.y - V1.y)) * dXCol;

    float dGdY21 = (float)(V2.RGB[1] - V1.RGB[1]) * dY21;
    float dGdY31 = (float)(V3.RGB[1] - V1.RGB[1]) * dY31;
    float dGdY31tmp = dGdY31;
    float dGdY32 = (float)(V3.RGB[1] - V2.RGB[1]) * dY32;
    float dGdX   = (float)((V3.RGB[1] - V1.RGB[1])*ceil(V2.y - V1.y) + (V1.RGB[1] - V2.RGB[1])*ceil(V3.y - V1.y)) * dXCol;

    float dBdY21 = (float)(V2.RGB[2] - V1.RGB[2]) * dY21;
    float dBdY31 = (float)(V3.RGB[2] - V1.RGB[2]) * dY31;
    float dBdY31tmp = dBdY31;
    float dBdY32 = (float)(V3.RGB[2] - V2.RGB[2]) * dY32;
    float dBdX   = (float)((V3.RGB[2] - V1.RGB[2])*ceil(V2.y - V1.y) + (V1.RGB[2] - V2.RGB[2])*ceil(V3.y - V1.y)) * dXCol;

    if (dXdY21 > dXdY31) {
        swap_data(dXdY21, dXdY31);
        swap_data(dRdY21, dRdY31);
        swap_data(dGdY21, dGdY31);
        swap_data(dBdY21, dBdY31);
    }

    int prestep = SUB_PIX(V1.y);
    int x;
    int x_end;
    float x_left = V1.x + prestep * dXdY21;
    float x_right = V1.x + prestep * dXdY31;
    int y = ceil(V1.y);
    float cRp = V1.RGB[0] + prestep * dRdY21;
    float cGp = V1.RGB[1] + prestep * dGdY21;
    float cBp = V1.RGB[2] + prestep * dBdY21;
    float cR, cG, cB;

    while (y < V2.y) {
        x_end = ceil(x_right);
        cR = ceil(cRp);
        cG = ceil(cGp);
        cB = ceil(cBp);

        if (x_left < x_right) {
            for (x=ceil(x_left); x<x_end; x++) {
                cR += dRdX;
                cG += dGdX;
                cB += dBdX;
                putPixel(x, y, cR, cG, cB, painter);
            }
        }
        else {
            x_end = x_left;
            for (x=ceil(x_right); x<x_end; x++) {
                cR += dRdX;
                cG += dGdX;
                cB += dBdX;
                putPixel(x, y, cR, cG, cB, painter);
            }
        }
        x_left  += dXdY21;
        x_right += dXdY31;
        cRp += dRdY21;
        cGp += dGdY21;
        cBp += dBdY21;
        y += 1.0;
    }

    dXdY31 = dXdY31tmp;
    dRdY31 = dRdY31tmp;
    dGdY31 = dGdY31tmp;
    dBdY31 = dBdY31tmp;
    if (dXdY32 < dXdY31) {
        swap_data(dXdY31, dXdY32);
        swap_data(dRdY31, dRdY32);
        swap_data(dGdY31, dGdY32);
        swap_data(dBdY31, dBdY32);
    }

    prestep = SUB_PIX(V2.y);
    if (V2.x > V1.x) {
        x_right = V2.x + prestep * dXdY31;
    }
    else {
        x_left = V2.x + SUB_PIX(V2.y) * dXdY32;
        cRp = V2.RGB[0] + prestep * dRdY32;
        cGp = V2.RGB[1] + prestep * dGdY32;
        cBp = V2.RGB[2] + prestep * dBdY32;
    }

    while (y < V3.y) {
        x_end    = ceil(x_right);
        cR = ceil(cRp);
        cG = ceil(cGp);
        cB = ceil(cBp);

        for (x=ceil(x_left); x<x_end; x++) {
            cR += dRdX;
            cG += dGdX;
            cB += dBdX;
            putPixel(x, y, cR, cG, cB, painter);
        }
        x_left  += dXdY32;
        x_right += dXdY31;
        cRp += dRdY32;
        cGp += dGdY32;
        cBp += dBdY32;
        y += 1.0;

    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QPixmap pixmap(WND_WIDTH, WND_HEIGHT);
    QPainter painter(&pixmap);
    QLabel windowLabel;

    TVertexCol A = {  40,  10, { 255,   0,   0} };
    TVertexCol B = { 290, 220, {   0, 255,   0} };
    TVertexCol C = {  80, 280, {   0,   0, 255} };

    TVertexCol D = {  40,  10, { 255,   0,   0} };
    TVertexCol E = { 290, 220, {   0, 255,   0} };
    TVertexCol F = { 270, -10, {   0,   0, 255} };

    QTimer t;
    int step = 100;
    QObject::connect(&t, &QTimer::timeout, [&]() {

        TVertexCol APrim, BPrim, CPrim;
        TVertexCol DPrim, EPrim, FPrim;
        float matrixRot[2];

        matrixRot[0] = qCos(step * M_PI / 100.0);                       // rotation around the z axis
        matrixRot[1] = qSin(step * M_PI / 100.0);

        APrim.x = WND_WIDTH/2  + A.x * matrixRot[0] - A.y * matrixRot[1];
        APrim.y = WND_HEIGHT/2 + A.x * matrixRot[1] + A.y * matrixRot[0];
        BPrim.x = WND_WIDTH/2  + B.x * matrixRot[0] - B.y * matrixRot[1];
        BPrim.y = WND_HEIGHT/2 + B.x * matrixRot[1] + B.y * matrixRot[0];
        CPrim.x = WND_WIDTH/2  + C.x * matrixRot[0] - C.y * matrixRot[1];
        CPrim.y = WND_HEIGHT/2 + C.x * matrixRot[1] + C.y * matrixRot[0];

        DPrim.x = WND_WIDTH/2  + D.x * matrixRot[0] - D.y * matrixRot[1];
        DPrim.y = WND_HEIGHT/2 + D.x * matrixRot[1] + D.y * matrixRot[0];
        EPrim.x = WND_WIDTH/2  + E.x * matrixRot[0] - E.y * matrixRot[1];
        EPrim.y = WND_HEIGHT/2 + E.x * matrixRot[1] + E.y * matrixRot[0];
        FPrim.x = WND_WIDTH/2  + F.x * matrixRot[0] - F.y * matrixRot[1];
        FPrim.y = WND_HEIGHT/2 + F.x * matrixRot[1] + F.y * matrixRot[0];

        APrim.RGB[0] = A.RGB[0]; //128 + 127 * qSin(step * M_PI / 50.0 + 0);
        APrim.RGB[1] = A.RGB[1];
        APrim.RGB[2] = A.RGB[2];
        BPrim.RGB[0] = B.RGB[0];
        BPrim.RGB[1] = B.RGB[1]; //128 + 127 * qSin(step * M_PI / 50.0 + (M_PI / 3));
        BPrim.RGB[2] = B.RGB[2];
        CPrim.RGB[0] = C.RGB[0];
        CPrim.RGB[1] = C.RGB[1];
        CPrim.RGB[2] = C.RGB[2]; //128 + 127 * qSin(step * M_PI / 50.0 + 2 * (M_PI / 3));

        DPrim.RGB[0] = 128 + 127 * qSin(step * M_PI / 50.0 + 0);
        DPrim.RGB[1] = D.RGB[1];
        DPrim.RGB[2] = D.RGB[2];
        EPrim.RGB[0] = E.RGB[0];
        EPrim.RGB[1] = 128 + 127 * qSin(step * M_PI / 50.0 + (M_PI / 3));
        EPrim.RGB[2] = E.RGB[2];
        FPrim.RGB[0] = F.RGB[0];
        FPrim.RGB[1] = F.RGB[1];
        FPrim.RGB[2] = 128 + 127 * qSin(step * M_PI / 50.0 + 2 * (M_PI / 3));

        pixmap.fill(Qt::black);
        drawTriangle(APrim, BPrim, CPrim, &painter);
        //drawTriangle(DPrim, EPrim, FPrim, &painter);
        windowLabel.setPixmap(pixmap);

        ++step;
        //t.stop();
    });
    t.start(10);

    windowLabel.setPixmap(pixmap);
    windowLabel.show();

    return a.exec();
}
