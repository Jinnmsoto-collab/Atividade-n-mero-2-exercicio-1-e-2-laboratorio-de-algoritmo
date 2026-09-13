#include <raylib.h>
#include <stdlib.h>
#include <stdbool.h>

#define W 1920
#define H 1080
#define CELL 40

typedef enum{BRONZE,PRATA,OURO,DIAMANTE}Tipo;
typedef enum{ARMA,POCAO,ESCUDO}TipoItem;

typedef struct{float dano;int alcance;}DadosArma;
typedef struct{int cura;}DadosPocao;
typedef struct{int absorcao;}DadosEscudo;
typedef union{DadosArma arma;DadosPocao pocao;DadosEscudo escudo;}DadosItem;

typedef struct{
    Vector2 pos;
    float raio;
    TipoItem tipo;
    DadosItem dados;
    bool coletado;
    float tempoColeta;
}Item;

typedef struct{
    Vector2 pos;
    float raio;
    int vida,dano,armadura;
}Jogador;

typedef struct{Vector2 pos,vel;float raio;Color cor;}Bola;

typedef struct{
    Vector2 pos;
    float raio;
    Tipo tipo;
    int valor;
    bool coletada;
    float tempoColeta;
}Moeda;

int valor(Tipo t){return t==BRONZE?5:t==PRATA?10:t==OURO?25:50;}

Color cor(Tipo t){
    return t==BRONZE?(Color){160,90,40,255}:t==PRATA?(Color){190,190,190,255}:t==OURO?GOLD:(Color){0,220,255,255};
}

Color corItem(Item *i){
    return i->tipo==ARMA?RED:i->tipo==ESCUDO?BLUE:i->dados.pocao.cura<0?PURPLE:GREEN;
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
    m->tipo=GetRandomValue(0,9)==0?DIAMANTE:GetRandomValue(BRONZE,OURO);
    m->valor=valor(m->tipo);
    m->coletada=false;m->tempoColeta=0;
}

void novoItem(Item *i){
    i->raio=14;
    i->pos=(Vector2){GetRandomValue(30,W-30),GetRandomValue(30,H-30)};
    i->tipo=GetRandomValue(ARMA,ESCUDO);
    i->coletado=false;i->tempoColeta=0;

    if(i->tipo==ARMA){
        i->dados.arma.dano=GetRandomValue(2,8);
        i->dados.arma.alcance=GetRandomValue(1,3);
    }else if(i->tipo==POCAO){
        i->dados.pocao.cura=GetRandomValue(10,30);
        if(GetRandomValue(0,9)<3)i->dados.pocao.cura=-GetRandomValue(5,20);
    }else i->dados.escudo.absorcao=GetRandomValue(2,8);
}

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

Item *criarItens(int n){
    Item *v=n>0?malloc(n*sizeof(Item)):NULL;
    if(!v)return NULL;
    for(int i=0;i<n;i++)novoItem(v+i);
    return v;
}

int **criarGrade(int l,int c){
    int **m=malloc(l*sizeof(int*));
    if(!m)return NULL;
    for(int i=0;i<l;i++){
        m[i]=calloc(c,sizeof(int));
        if(!m[i]){
            while(i--)free(m[i]);
            free(m);return NULL;
        }
    }
    return m;
}

void liberarGrade(int **m,int l){
    if(m){for(int i=0;i<l;i++)free(m[i]);free(m);}
}

void atualizar(Bola *b){
    b->pos.x+=b->vel.x;b->pos.y+=b->vel.y;

    if(b->pos.x<=b->raio||b->pos.x>=W-b->raio){
        b->pos.x=b->pos.x<=b->raio?b->raio:W-b->raio;
        b->vel.x*=-1;
    }

    if(b->pos.y<=b->raio||b->pos.y>=H-b->raio){
        b->pos.y=b->pos.y<=b->raio?b->raio:H-b->raio;
        b->vel.y*=-1;
    }
}

void mover(Jogador *j){
    if(IsKeyDown(KEY_W))j->pos.y-=5;
    if(IsKeyDown(KEY_S))j->pos.y+=5;
    if(IsKeyDown(KEY_A))j->pos.x-=5;
    if(IsKeyDown(KEY_D))j->pos.x+=5;

    if(j->pos.x<j->raio)j->pos.x=j->raio;
    if(j->pos.x>W-j->raio)j->pos.x=W-j->raio;
    if(j->pos.y<j->raio)j->pos.y=j->raio;
    if(j->pos.y>H-j->raio)j->pos.y=H-j->raio;
}

bool perto(Vector2 a,Vector2 b,float r){
    float x=a.x-b.x,y=a.y-b.y;
    return x*x+y*y<=r*r;
}

bool coletarMoeda(Moeda *m,Jogador *j){
    if(!m->coletada&&perto(m->pos,j->pos,m->raio+j->raio)){
        m->coletada=true;m->tempoColeta=GetTime();return true;
    }
    return false;
}

bool coletarItem(Item *i,Jogador *j){
    if(i->coletado||!perto(i->pos,j->pos,i->raio+j->raio))return false;

    switch(i->tipo){
        case ARMA:j->dano+=(int)i->dados.arma.dano;break;
        case POCAO:j->vida+=i->dados.pocao.cura;if(j->vida<0)j->vida=0;break;
        case ESCUDO:j->armadura+=i->dados.escudo.absorcao;break;
    }

    i->coletado=true;i->tempoColeta=GetTime();
    return true;
}

void adicionarBola(Bola **v,int *n){
    Bola *p=realloc(*v,(*n+1)*sizeof(Bola));
    if(p){*v=p;novaBola(p+(*n));(*n)++;}
}

void adicionarMoeda(Moeda **v,int *n){
    Moeda *p=realloc(*v,(*n+1)*sizeof(Moeda));
    if(p){*v=p;novaMoeda(p+(*n));(*n)++;}
}

void adicionarItem(Item **v,int *n){
    Item *p=realloc(*v,(*n+1)*sizeof(Item));
    if(p){*v=p;novoItem(p+(*n));(*n)++;}
}

void desenharGrade(int **g,int l,int c){
    for(int i=0;i<l;i++)
        for(int j=0;j<c;j++){
            DrawRectangle(j*CELL,i*CELL,CELL,CELL,
                g[i][j]?(Color){80,160,100,255}:(Color){35,35,45,255});
            DrawRectangleLines(j*CELL,i*CELL,CELL,CELL,(Color){70,70,80,255});
        }
}

void desenharItem(Item *i){
    if(i->coletado)return;
    DrawCircleV(i->pos,i->raio,corItem(i));
    DrawText(i->tipo==ARMA?"A":i->tipo==POCAO?"P":"E",i->pos.x-6,i->pos.y-9,18,WHITE);
}

int main(void){
    InitWindow(W,H,"Atividade 3");
    SetTargetFPS(60);

    int l=H/CELL,c=W/CELL,nb=12,nm=8,ni=10,pontos=0,cm=0,ci=0;
    int **grade=criarGrade(l,c);
    Bola *bolas=criarBolas(nb);
    Moeda *moedas=criarMoedas(nm);
    Item *itens=criarItens(ni);

    Jogador j={{W/2.0f,H/2.0f},18,100,10,0};

    if(!grade||!bolas||!moedas||!itens){
        liberarGrade(grade,l);free(bolas);free(moedas);free(itens);
        CloseWindow();return 1;
    }

    while(!WindowShouldClose()){
        mover(&j);

        if(IsKeyPressed(KEY_SPACE))adicionarBola(&bolas,&nb);
        if(IsKeyPressed(KEY_M))adicionarMoeda(&moedas,&nm);
        if(IsKeyPressed(KEY_I))adicionarItem(&itens,&ni);

        if(IsKeyPressed(KEY_BACKSPACE)&&nb>1){
            Bola *p=realloc(bolas,(nb-1)*sizeof(Bola));
            if(p)bolas=p,nb--;
        }

        if(IsKeyPressed(KEY_N)&&nm>1){
            Moeda *p=realloc(moedas,(nm-1)*sizeof(Moeda));
            if(p)moedas=p,nm--;
        }

        for(int i=0;i<nm;i++)
            if(moedas[i].coletada&&GetTime()-moedas[i].tempoColeta>=3)
                novaMoeda(moedas+i);

        for(int i=0;i<ni;i++)
            if(itens[i].coletado&&GetTime()-itens[i].tempoColeta>=3)
                novoItem(itens+i);

        for(int i=0;i<nb;i++){
            atualizar(bolas+i);
            int x=bolas[i].pos.x/CELL,y=bolas[i].pos.y/CELL;
            if(x>=0&&x<c&&y>=0&&y<l)grade[y][x]=1;
        }

        for(int i=0;i<nm;i++)
            if(coletarMoeda(moedas+i,&j))
                pontos+=moedas[i].valor,cm++;

        for(int i=0;i<ni;i++)
            if(coletarItem(itens+i,&j))ci++;

        int visitadas=0;
        for(int i=0;i<l;i++)
            for(int k=0;k<c;k++)
                visitadas+=grade[i][k];

        BeginDrawing();
        ClearBackground(BLACK);
        desenharGrade(grade,l,c);

        for(int i=0;i<nm;i++)
            if(!moedas[i].coletada)
                DrawCircleV(moedas[i].pos,moedas[i].raio,cor(moedas[i].tipo));

        for(int i=0;i<ni;i++)desenharItem(itens+i);
        for(int i=0;i<nb;i++)DrawCircleV(bolas[i].pos,bolas[i].raio,bolas[i].cor);

        DrawCircleV(j.pos,j.raio,ORANGE);
        DrawText("J",j.pos.x-6,j.pos.y-10,20,WHITE);

        DrawRectangle(10,10,430,220,(Color){0,0,0,210});
        DrawText(TextFormat("Vida: %d | Dano: %d | Armadura: %d",j.vida,j.dano,j.armadura),20,20,20,WHITE);
        DrawText(TextFormat("Bolas: %d | Celulas: %d",nb,visitadas),20,50,20,WHITE);
        DrawText(TextFormat("Moedas: %d | Coletadas: %d",nm,cm),20,80,20,WHITE);
        DrawText(TextFormat("Pontos: %d | Itens: %d",pontos,ci),20,110,20,YELLOW);
        DrawText("WASD mover | ESPACO +bola | BACKSPACE -bola",20,145,15,LIGHTGRAY);
        DrawText("M +moeda | N -moeda | I +item",20,170,15,LIGHTGRAY);
        DrawText("A arma | P pocao | E escudo",20,195,15,LIGHTGRAY);

        EndDrawing();
    }
/*coloquei um J pra mostrar qual bolinha é o jogador*/
    free(bolas);free(moedas);free(itens);
    liberarGrade(grade,l);
    CloseWindow();
    return 0;
}
