#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>

#define W 1920
#define H 1080
#define CELL 40

typedef enum { BRONZE, PRATA, OURO, DIAMANTE } Tipo;

typedef struct {
    Vector2 pos, vel;
    float raio;
    Color cor;
} Bola;

typedef struct {
    Vector2 pos;
    float raio;
    Tipo tipo;
    int valor;
    bool coletada;
    float tempoColeta;
} Moeda;

int valor(Tipo t){return t==BRONZE?5:t==PRATA?10:t==OURO?25:50;}
Color cor(Tipo t){
    return t==BRONZE?(Color){160,90,40,255}:
           t==PRATA?(Color){190,190,190,255}:
           t==OURO?GOLD:(Color){0,220,255,255};
}

void novaBola(Bola *b){
    b->raio=12;
    b->pos=(Vector2){GetRandomValue(12,W-12),GetRandomValue(12,H-12)};
    b->vel=(Vector2){GetRandomValue(-3,3),GetRandomValue(-3,3)};
    if(!b->vel.x)b->vel.x=2;
    if(!b->vel.y)b->vel.y=2;
    b->cor=(Color){GetRandomValue(50,255),GetRandomValue(50,255),GetRandomValue(50,255),255};
}

void novaMoeda(Moeda *m){
    m->raio=10;
    m->pos=(Vector2){GetRandomValue(10,W-10),GetRandomValue(10,H-10)};
    if(GetRandomValue(0,9)==0)
        m->tipo=DIAMANTE;
    else
        m->tipo=GetRandomValue(BRONZE,OURO);
    m->valor=valor(m->tipo);
    m->coletada=false;
    m->tempoColeta=0;
}
/*durante essa parte aqui da moeda, demorei pra entender porque ele mostrava prata mesmo sem a adição enquanto eu fazia uns testes, até perceber q prata é = 1 e por isso sorteava mesmo assim, enquanto eu queria testar os outros valores kkkkkk*/
Bola *criarBolas(int n){
    Bola *v=n>0?malloc(n*sizeof(Bola)):NULL;
    if(!v)return NULL;
    for(int i=0;i<n;i++)novaBola(v+i);
    return v;
}

Moeda *criarMoedas(int n){
    Moeda *v=n>0?malloc(n*sizeof(Moeda)):NULL;
    if(!v)return NULL;
    for(int i=0;i<n;i++)novaMoeda(v+i);
    return v;
}

int **criarGrade(int l,int c){
    int **m=malloc(l*sizeof(int*));
    if(!m)return NULL;
    for(int i=0;i<l;i++){
        m[i]=calloc(c,sizeof(int));
        if(!m[i]){
            while(i--)free(m[i]);
            free(m);
            return NULL;
        }
    }
    return m;
}

void liberarGrade(int **m,int l){
    if(m){for(int i=0;i<l;i++)free(m[i]);free(m);}
}

void atualizar(Bola *b){
    b->pos.x+=b->vel.x;
    b->pos.y+=b->vel.y;

    if(b->pos.x<=b->raio||b->pos.x>=W-b->raio){
        b->pos.x=b->pos.x<=b->raio?b->raio:W-b->raio;
        b->vel.x*=-1;
    }
    if(b->pos.y<=b->raio||b->pos.y>=H-b->raio){
        b->pos.y=b->pos.y<=b->raio?b->raio:H-b->raio;
        b->vel.y*=-1;
    }
}

bool coletar(Moeda *m,Bola *b){
    if(m->coletada)return false;
    float x=m->pos.x-b->pos.x,y=m->pos.y-b->pos.y,r=m->raio+b->raio;
    if(x*x+y*y<=r*r){
        m->coletada=true;
        m->tempoColeta=GetTime();
        return true;
    }
    return false;
}

void adicionarBola(Bola **v,int *n){
    Bola *p=realloc(*v,(*n+1)*sizeof(Bola));
    if(!p)return;
    *v=p;
    novaBola(*v+(*n)++);
}

void adicionarMoeda(Moeda **v,int *n){
    Moeda *p=realloc(*v,(*n+1)*sizeof(Moeda));
    if(!p)return;
    *v=p;
    novaMoeda(*v+(*n)++);
}

void desenharGrade(int **g,int l,int c){
    for(int i=0;i<l;i++)
        for(int j=0;j<c;j++){
            DrawRectangle(j*CELL,i*CELL,CELL,CELL,
                g[i][j]?(Color){80,160,100,255}:(Color){35,35,45,255});
            DrawRectangleLines(j*CELL,i*CELL,CELL,CELL,(Color){70,70,80,255});
        }
}

int main(void){
    InitWindow(W,H,"Ponteiros e Memoria");
    SetTargetFPS(60);

    int l=H/CELL,c=W/CELL,nb=12,nm=8,pontos=0,coletadas=0;
    int **grade=criarGrade(l,c);
    Bola *bolas=criarBolas(nb);
    Moeda *moedas=criarMoedas(nm);

    if(!grade||!bolas||!moedas){
        liberarGrade(grade,l);free(bolas);free(moedas);CloseWindow();return 1;
    }

    while(!WindowShouldClose()){
        if(IsKeyPressed(KEY_SPACE))adicionarBola(&bolas,&nb);
        if(IsKeyPressed(KEY_M))adicionarMoeda(&moedas,&nm);

        if(IsKeyPressed(KEY_BACKSPACE)&&nb>1){
            Bola *p=realloc(bolas,(nb-1)*sizeof(Bola));
            if(p)bolas=p,nb--;
        }

        if(IsKeyPressed(KEY_N)&&nm>1){
            Moeda *p=realloc(moedas,(nm-1)*sizeof(Moeda));
            if(p)moedas=p,nm--;
        }

        for(int i=0;i<nm;i++){
            Moeda *m=moedas+i;
            if(m->coletada&&GetTime()-m->tempoColeta>=3)novaMoeda(m);
        }

        for(int i=0;i<nb;i++){
            Bola *b=bolas+i;
            atualizar(b);

            int x=b->pos.x/CELL,y=b->pos.y/CELL;
            if(x>=0&&x<c&&y>=0&&y<l)grade[y][x]=1;

            for(int j=0;j<nm;j++)
                if(coletar(moedas+j,b))
                    pontos+=(moedas+j)->valor,coletadas++;
        }

        int visitadas=0;
        for(int i=0;i<l;i++)
            for(int j=0;j<c;j++)
                visitadas+=grade[i][j];

        BeginDrawing();
        ClearBackground(BLACK);
        desenharGrade(grade,l,c);

        for(int i=0;i<nm;i++)
            if(!moedas[i].coletada)
                DrawCircleV(moedas[i].pos,moedas[i].raio,cor(moedas[i].tipo));

        for(int i=0;i<nb;i++)
            DrawCircleV(bolas[i].pos,bolas[i].raio,bolas[i].cor);

        DrawRectangle(10,10,400,145,(Color){0,0,0,210});
        DrawText(TextFormat("Bolas: %d",nb),20,20,20,WHITE);
        DrawText(TextFormat("Celulas visitadas: %d",visitadas),20,45,20,WHITE);
        DrawText(TextFormat("Moedas: %d | Coletadas: %d",nm,coletadas),20,70,20,WHITE);
        DrawText(TextFormat("Pontuacao: %d",pontos),20,95,20,YELLOW);
        DrawText("ESPACO +bola | BACKSPACE -bola",20,120,15,LIGHTGRAY);
        DrawText("M +moeda | N -moeda",20,140,15,LIGHTGRAY);
        EndDrawing();
    }
/* Eu percebi q tinha linhas desnecessárias e acabei comprimindo elas em uma só, além disso deixei as váriaveis com nomes completos pra melhor entendimento, eu dei uma mudadinha na resolução pra ficar em tela cheia no meu pc pq tava me dando agonia, e usei um sistema de verificação q eu achei na pesquisa de como usar malloc direito, pq eu n tinha entendido e só tinha testado até funcionar.*/
    free(bolas);
    free(moedas);
    liberarGrade(grade,l);
    CloseWindow();
    return 0;
}