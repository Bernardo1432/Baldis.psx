#include <sys/types.h>
#include <libgte.h>
#include <libgpu.h>
#include <libetc.h>

#define VIDEO_LARGURA 320
#define VIDEO_ALTURA  240
#define MAPA_LARGURA 16
#define MAPA_ALTURA 16
#define FOV 0.78539f
#define PROFUNDIDADE_MAX 12.0f
#define TOTAL_CADERNOS 7

typedef struct { DRAWENV draw; DISPENV disp; } DB;
DB db[2];
int db_atual = 0;
unsigned char buffer_controle[34];

int idioma = 0; 
int telaAtual = 0; 
int posicaoMenu = 0;

float playerX = 8.0f; float playerY = 4.0f; float playerA = 0.0f;
int cadernosColetados = 0; int stamina = 100; int estaCorrendo = 0;
int frameContador = 0; int minutos = 0; int segundos = 0;
int temBsoda = 0; int temChocolate = 0; int temTesoura = 0; int temChave = 0;
float baldiX = 2.0f; float baldiY = 2.0f; int baldiVel = 0;
float diretorX = 8.0f; float diretorY = 8.0f; int diretorVel = 0;

char* textosMenu[3][4] = {
    {"Iniciar Jogo", "Configuracoes", "Creditos", "Sair Jogo"},
    {"Start Game", "Options", "Credits", "Exit Game"},
    {"Iniciar Juego", "Configuraciones", "Creditos", "Salir Juego"}
};
char* textosOptions[3][2] = {
    {"Idioma: Portugues", "Voltar"}, {"Language: English", "Back"}, {"Idioma: Espanol", "Volver"}
};

char mapa[MAPA_ALTURA][MAPA_LARGURA] = {
    "1111111111111111", "1C000100001000C1", "1000010000100001", "1110110000110111",
    "1000000000000001", "1011111001111101", "1010001001000101", "1000K00000000001",
    "1010001001000101", "1011111001111101", "1000000000000001", "1110110000110111",
    "1000010000100001", "1C000100001000C1", "1111111111111111"
};

void InicializarSistemaPS1() {
    ResetGraph(0);
    SetDefDrawEnv(&db[0].draw, 0, 0, VIDEO_LARGURA, VIDEO_ALTURA);
    SetDefDispEnv(&db[0].disp, 0, 0, VIDEO_LARGURA, VIDEO_ALTURA);
    SetDefDrawEnv(&db[1].draw, 0, VIDEO_ALTURA, VIDEO_LARGURA, VIDEO_ALTURA);
    SetDefDispEnv(&db[1].disp, 0, VIDEO_ALTURA, VIDEO_LARGURA, VIDEO_ALTURA);
    setRGB0(&db[0].draw, 0, 0, 0); setRGB0(&db[1].draw, 0, 0, 0);
    PutDrawEnv(&db[0].draw); PutDispEnv(&db[0].disp);
    InitPAD(buffer_controle, 34, buffer_controle, 34); StartPAD();
    FntLoad(960, 256); FntOpen(8, 8, 304, 224, 0, 1024);
}

void InverterBuffersPS1() {
    DrawSync(0); VSync(0);
    db_atual = !db_atual;
    PutDrawEnv(&db[db_atual].draw); PutDispEnv(&db[db_atual].disp);
}

void ResetarPartida() {
    playerX = 8.0f; playerY = 4.0f; playerA = 0.0f;
    baldiX = 2.0f; baldiY = 2.0f; diretorX = 8.0f; diretorY = 8.0f;
    cadernosColetados = 0; stamina = 100; minutos = 0; segundos = 0; frameContador = 0;
    temBsoda = 0; temChocolate = 0; temTesoura = 0; temChave = 0;
}

int main() {
    int direcionalTrava = 0; int botaoTrava = 0;
    InicializarSistemaPS1();
    while (1) {
        unsigned short pad = ~((unsigned short*)(buffer_controle));
        if (telaAtual == 0 || telaAtual == 1) {
            FntPrint("     BALDI'S BASICS - PSX MENU     \n");
            FntPrint("===================================\n\n");
            int limite = (telaAtual == 0) ? 4 : 2;
            if (telaAtual == 0) {
                int i; for(i=0; i<4; i++) FntPrint(i==posicaoMenu ? " -> [ %s ] <-\n" : "    %s\n", textosMenu[idioma][i]);
            } else {
                int i; for(i=0; i<2; i++) FntPrint(i==posicaoMenu ? " -> [ %s ] <-\n" : "    %s\n", textosOptions[idioma][i]);
            }
            if (!direcionalTrava) {
                if (pad & PADLup) { posicaoMenu = (posicaoMenu - 1 + limite) % limite; direcionalTrava = 1; }
                if (pad & PADLdown) { posicaoMenu = (posicaoMenu + 1) % limite; direcionalTrava = 1; }
            }
            if (!(pad & (PADLup | PADLdown))) direcionalTrava = 0;
            if (!botaoTrava && (pad & PADRdown)) {
                botaoTrava = 1;
                if (telaAtual == 0) {
                    if (posicaoMenu == 0) { telaAtual = 3; ResetarPartida(); }
                    else if (posicaoMenu == 1) { telaAtual = 1; posicaoMenu = 0; }
                    else if (posicaoMenu == 2) telaAtual = 2;
                    else if (posicaoMenu == 3) ResetGraph(3);
                } else {
                    if (posicaoMenu == 0) idioma = (idioma + 1) % 3;
                    else { telaAtual = 0; posicaoMenu = 1; }
                }
            }
            if (!botaoTrava && (pad & PADRup)) { botaoTrava = 1; if (telaAtual == 1) { telaAtual = 0; posicaoMenu = 0; } }
            if (!(pad & (PADRdown | PADRup))) botaoTrava = 0;
        }
        else if (telaAtual == 2) {
            FntPrint("     BALDI'S BASICS - CREDITS      \n");
            FntPrint("===================================\n\n");
            FntPrint(idioma == 0 ? " Criado para rodar no Game Stick\n" : " Made for Game Stick PS1!\n");
            FntPrint("\n Pressione TRIANGULO para voltar...");
            if (pad & PADRup) { telaAtual = 0; posicaoMenu = 2; }
        }
        else if (telaAtual == 3) {
            frameContador++;
            if (frameContador >= 60) { frameContador = 0; segundos++; if (segundos >= 60) { segundos = 0; minutos++; } }
            baldiVel++;
            if (baldiVel >= (8 - cadernosColetados)) {
                baldiVel = 0;
                if (baldiX < playerX && mapa[(int)baldiY][(int)baldiX+1] != '1') baldiX += 1.0f;
                else if (baldiX > playerX && mapa[(int)baldiY][(int)baldiX-1] != '1') baldiX -= 1.0f;
                if (baldiY < playerY && mapa[(int)baldiY+1][(int)baldiX] != '1') baldiY += 1.0f;
                else if (baldiY > playerY && mapa[(int)baldiY-1][(int)baldiX] != '1') baldiY -= 1.0f;
            }
            if ((int)playerX == (int)baldiX && (int)playerY == (int)baldiY) { telaAtual = 4; }
            if (cadernosColetados >= TOTAL_CADERNOS && (playerX < 2 || playerX > 14 || playerY < 2 || playerY > 14)) { telaAtual = 5; }
            if ((pad & PADRright) && stamina > 5) { estaCorrendo = 1; stamina -= 2; } 
            else { estaCorrendo = 0; if (stamina < 100) stamina++; }
            float velFisica = estaCorrendo ? 0.16f : 0.08f;
            if (pad & PADLleft)  playerA -= 0.05f;
            if (pad & PADLright) playerA += 0.05f;
            if (pad & PADLup) {
                float nx = playerX + sin(playerA) * velFisica; float ny = playerY + cos(playerA) * velFisica;
                if (mapa[(int)ny][(int)nx] != '1') { playerX = nx; playerY = ny; }
            }
            if (pad & PADLdown) {
                float nx = playerX - sin(playerA) * velFisica; float ny = playerY - cos(playerA) * velFisica;
                if (mapa[(int)ny][(int)nx] != '1') { playerX = nx; playerY = ny; }
            }
            if ((pad & PADRleft) && temBsoda) { temBsoda = 0; baldiX = 2.0f; baldiY = 2.0f; }
            if ((pad & PADRup) && temChocolate) { temChocolate = 0; stamina = 100; }
            char bloco = mapa[(int)playerY][(int)playerX];
            if (bloco == 'C') { mapa[(int)playerY][(int)playerX] = '0'; cadernosColetados++; }
            else if (bloco == 'S') { temBsoda = 1; mapa[(int)playerY][(int)playerX] = '0'; }
            else if (bloco == 'Z') { temChocolate = 1; mapa[(int)playerY][(int)playerX] = '0'; }
            FntPrint(" CADERNOS: %d/%d | %02d:%02d | STAMINA: %d%%\n", cadernosColetados, TOTAL_CADERNOS, minutos, segundos, stamina);
            FntPrint("-----------------------------------\n");
            int x; for (x = 0; x < 40; x++) {
                float anguloRaio = (playerA - 0.392f) + ((float)x / 40.0f) * 0.785f;
                float dist = 0.0f; int bateu = 0; char alvo = ' ';
                float oX = sin(anguloRaio); float oY = cos(anguloRaio);
                while (!bateu && dist < PROFUNDIDADE_MAX) {
                    dist += 0.2f; int tx = (int)(playerX + oX * dist); int ty = (int)(playerY + oY * dist);
                    if (tx < 0 || tx >= MAPA_LARGURA || ty < 0 || ty >= MAPA_ALTURA) { bateu = 1; dist = PROFUNDIDADE_MAX; }
                    else {
                        char celula = mapa[ty][tx];
                        if (celula == '1' || celula == 'C' || celula == 'S') { bateu = 1; alvo = celula; }
                        else if ((int)baldiX == tx && (int)baldiY == ty) { bateu = 1; alvo = 'B'; }
                    }
                }
                dist = dist * cos(anguloRaio - playerA);
                if (alvo == '1') FntPrint(dist < 4.0f ? "||" : dist < 8.0f ? "!!" : "..");
                else if (alvo == 'B') FntPrint("BB");
                else if (alvo == 'C') FntPrint("CC");
                else FntPrint("  ");
                if (x == 19 || x == 39) FntPrint("\n");
            }
        }
        else if (telaAtual == 4 || telaAtual == 5) {
            FntPrint(telaAtual == 4 ? "    !!! GAME OVER - BALDI !!!    \n" : "    !!! VICTORY - YOU WIN !!!    \n");
            FntPrint("===================================\n\n");
            FntPrint(idioma == 0 ? " Tempo Sobrevivido: %02d:%02d\n" : " Time Survived: %02d:%02d\n", minutos, segundos);
            FntPrint("\n Pressione CROSS (X) para Menu...");
            if (pad & PADRdown) { telaAtual = 0; posicaoMenu = 0; }
        }
        FntFlush(-1); InverterBuffersPS1();
    }
    return 0;
}
