#include <windows.h>
#include <gl/gl.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <math.h>

LRESULT CALLBACK WindowProc(HWND, UINT, WPARAM, LPARAM);
void EnableOpenGL(HWND hwnd, HDC*, HGLRC*);
void DisableOpenGL(HWND, HDC, HGLRC);
void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius);
void Desenhar_Retangulos(float x, float y);
void Teclado(float *y);
void Inteligencia_Artificial(float *y, float ybola);
void Colisao(float xbola, float ybola, float yplayer, float yjogo, float *angulo, int *mudar, int *score);
void Barreira(float xbola, float ybola, float *angulo);
HWND texto;


int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    WNDCLASSEX wcex;
    HWND hwnd;
    HDC hDC;
    HGLRC hRC;
    MSG msg;
    BOOL bQuit = FALSE;
    float yplayer=0, yjogo=0, xbola=0, ybola=0, angulo=0;
    int mudar=0, score=0, scorebuffer=0;
    char placar[30];

    /* register window class */
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_OWNDC;
    wcex.lpfnWndProc = WindowProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(NULL, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = "GLSample";
    wcex.hIconSm = LoadIcon(NULL, IDI_APPLICATION);;

    srand(time(NULL));
    angulo= rand()%21+70;
    float val= M_PI/180;
    sprintf(placar,"Score: %d",score);

    if (!RegisterClassEx(&wcex))
        return 0;

    /* create main window */
    hwnd = CreateWindowEx(0,
                          "GLSample",
                          "OpenGL Sample",
                          WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
                          500,
                          100,
                          600,
                          600,
                          NULL,
                          NULL,
                          hInstance,
                          NULL);

    ShowWindow(hwnd, nCmdShow);
    texto=CreateWindow("STATIC",
                        placar,
                        WS_VISIBLE | WS_CHILD,
                        40,20,82,16,
                        hwnd, NULL, NULL, NULL);

    /* enable OpenGL for the window */
    EnableOpenGL(hwnd, &hDC, &hRC);

    /* program main loop */
    while (!bQuit)
    {
        /* check for messages */
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            /* handle or dispatch messages */
            if (msg.message == WM_QUIT)
            {
                bQuit = TRUE;
            }
            else
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
            /* OpenGL animation code goes here */

            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
            glClear(GL_COLOR_BUFFER_BIT);

            glPushMatrix();
            glColor3f(1.0,1.0,1.0);


//
            //sprintf(placar,"Score: %d",score);



            xbola=xbola+sin(angulo*val)/40;
            ybola=ybola+cos(angulo*val)/40;

            Desenhar_Retangulos(-0.8,yplayer);
            Desenhar_Retangulos(0.8,yjogo);
            drawFilledCircle(xbola,ybola,0.03);
            Teclado(&yplayer);
            if(mudar==0)
            {
                Inteligencia_Artificial(&yjogo,ybola);
            }

            Colisao(xbola, ybola, yplayer, yjogo, &angulo, &mudar, &score);

            Barreira(xbola, ybola, &angulo);

//
            glPopMatrix();

            SwapBuffers(hDC);
            glFlush();

            Sleep (100);
            if(score!=scorebuffer)
            {
                sprintf(placar,"Score: %d",score);
                texto=CreateWindow("STATIC",
                                    placar,
                                    WS_VISIBLE | WS_CHILD,
                                    40,20,82,16,
                                    hwnd, NULL, NULL, NULL);
                scorebuffer=score;
            }
        }
    }

    /* shutdown OpenGL */
    DisableOpenGL(hwnd, hDC, hRC);

    /* destroy the window explicitly */
    DestroyWindow(hwnd);

    return msg.wParam;
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_CLOSE:
            PostQuitMessage(0);
        break;

        case WM_CTLCOLORSTATIC:
            SetTextColor((HDC)wParam,RGB(255,255,255));
            SetBkColor((HDC)wParam,RGB(0,0,0));
            SetBkMode((HDC)wParam,OPAQUE);
            return GetClassLong(hwnd, GCL_HBRBACKGROUND);

        case WM_DESTROY:
            return 0;

        case WM_KEYDOWN:
        {
            switch (wParam)
            {
                case VK_ESCAPE:
                    PostQuitMessage(0);
                break;
            }
        }
        break;

        default:
            return DefWindowProc(hwnd, uMsg, wParam, lParam);
    }

    return 0;
}

void EnableOpenGL(HWND hwnd, HDC* hDC, HGLRC* hRC)
{
    PIXELFORMATDESCRIPTOR pfd;

    int iFormat;

    /* get the device context (DC) */
    *hDC = GetDC(hwnd);

    /* set the pixel format for the DC */
    ZeroMemory(&pfd, sizeof(pfd));

    pfd.nSize = sizeof(pfd);
    pfd.nVersion = 1;
    pfd.dwFlags = PFD_DRAW_TO_WINDOW |
                  PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER;
    pfd.iPixelType = PFD_TYPE_RGBA;
    pfd.cColorBits = 24;
    pfd.cDepthBits = 16;
    pfd.iLayerType = PFD_MAIN_PLANE;

    iFormat = ChoosePixelFormat(*hDC, &pfd);

    SetPixelFormat(*hDC, iFormat, &pfd);

    /* create and enable the render context (RC) */
    *hRC = wglCreateContext(*hDC);

    wglMakeCurrent(*hDC, *hRC);
}

void DisableOpenGL (HWND hwnd, HDC hDC, HGLRC hRC)
{
    wglMakeCurrent(NULL, NULL);
    wglDeleteContext(hRC);
    ReleaseDC(hwnd, hDC);
}

void drawFilledCircle(GLfloat x, GLfloat y, GLfloat radius){
	int i;
	int triangleAmount = 20; //# of triangles used to draw circle

	//GLfloat radius = 0.8f; //radius
	GLfloat twicePi = 2.0f * M_PI;

	glBegin(GL_TRIANGLE_FAN);
		glVertex2f(x, y); // center of circle
		for(i = 0; i <= triangleAmount;i++) {
			glVertex2f(
		            x + (radius * cos(i *  twicePi / triangleAmount)),
			    y + (radius * sin(i * twicePi / triangleAmount))
			);
		}
	glEnd();
}

void Desenhar_Retangulos(float x, float y)
{
    glBegin(GL_QUADS);
        //glColor3f(1,0,0);
        glVertex2f(x+0.03,y+0.25);
        glVertex2f(x-0.03,y+0.25);
        glVertex2f(x-0.03,y-0.25);
        glVertex2f(x+0.03,y-0.25);


    glEnd();
}

void Teclado(float *y)
{
    if ((GetAsyncKeyState(VK_UP))||(GetAsyncKeyState(0x57)))
    {
        if(*y+0.8<=1.3)
        {
           *y+=0.01;
        }

    }
    else
    if ((GetAsyncKeyState(VK_DOWN))||(GetAsyncKeyState(0x53)))
    {
        if(*y-0.8>=-1.5)
        {
           *y-=0.01;
        }
    }

}

void Inteligencia_Artificial(float *y, float ybola)
{
    if(ybola > (*y+0.1))
    {
        if(*y+0.8<=1.3)
        {
           *y+=0.02;
        }
    }
    else
    if(ybola < (*y-0.1))
    {
        if(*y-0.8>=-1.5)
        {
           *y-=0.02;
        }
    }
}

void Colisao(float xbola, float ybola, float yplayer, float yjogo, float *angulo, int *mudar, int *score)
{

    if((xbola+0.04>=0.77)&&((ybola<=yjogo+0.25)&&(ybola>=yjogo-0.25)))
    {
        if(ybola>yjogo)
        {
            *angulo= 270 + rand()%31+11;
        }
        else
        if(ybola<yjogo)
        {
            *angulo= 270 - rand()%31+11;
        }
        *mudar=1;
    }
    else
    if((xbola-0.04<=-0.77)&&((ybola<=yplayer+0.25)&&(ybola>=yplayer-0.25)))
    {
        if(ybola>yplayer)
        {
            *angulo= 90 - rand()%31+11;
        }
        else
        if(ybola<yplayer)
        {
            *angulo= 90 + rand()%31+11;
        }
        *mudar=0;
        *score+=40;
    }

}

void Barreira(float xbola, float ybola, float *angulo)
{
    float mudar;
    if(ybola+0.03>=0.75)
    {
        if(*angulo<90)
        {
            *angulo= 90 + rand()%31+11;
        }
        else
        if(*angulo>270)
        {
            *angulo= 270 - rand()%31+11;
        }
    }
    else
    if(ybola-0.03<=-0.95)
    {
        if(*angulo>180)
        {
            *angulo= 180 + rand()%31+111;
        }
        else
        if(*angulo<180)
        {
            *angulo= 90 - rand()%31+11;
        }
    }
    if((xbola>=0.8)||(xbola<=-0.8))
    {
        exit(0);
    }

}
