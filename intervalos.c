#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#define MIN_SIZE 50

// CONTAGEM PRA EVITAR ENFIAR ESPAÇO ONDE NÃO DEVE
int conta_pulos=0;

/* DUAS ESTRUTURAS PRINCIPAIS: */

typedef struct{
	int xi,yi,xf,yf;
} segment;

typedef struct{
	int x,y;
	int s; // ÍNDICE NO VETOR DE SEGMENTOS
	int arr_x; // ÍNDICE NO VETOR X (só vai ser usado na construção da Range)
} point;

// ÁRVORE RANGE (PARA QUEM TEM O ENDPOINT CONTIDO NA COISA TODA)

// A árvore X =================================

typedef struct x_node{
	point p;
	struct x_node *left;
	struct x_node *right;
	struct x_node *parent; // <<< NEM SEI SE VOU CHEGAR A USAR ESSE PONTEIRO, MAS JUST IN CASE
	struct y_node *y_tree; // A ÁRVORE Y PENDURADA (corresponde ao conjunto canônico)
} x_node;

// A estrutura secundária (a árvore Y) =================================

typedef struct y_node{
	point p;
	struct y_node *left;
	struct y_node *right;
	struct y_node *parent; // <<< NEM SEI SE VOU CHEGAR A USAR ESSE PONTEIRO, MAS JUST IN CASE
} y_node;

// ÁRVORE DE INTERVALOS COMO VISTA EM SALA (PARA QUEM SÓ DÁ UMA RASPADINHA NO RETÂNGULO)

// As árvores em si =================================

typedef struct{
	int ini,fim;
} interval;

typedef struct s_node{
	interval i; // DE ONDE ATÉ ONDE VAI O NODO
	int *lista; // LISTA DE SEGMENTOS (SEGUNDO SEUS ÍNDICES NO VETOR seg), YAY
	int s_array_size; // Tamanho da lista de segmentos
	int s_alloc_memory; // Memória alocada pra lista de segmentos
	struct s_node *left;
	struct s_node *right;
	// ACABEI NÃO USANDO PARENT PRA NADA NA RANGE, MAS TÔ COM PREGUIÇA DE TIRAR DE LÁ
} s_node;

// Vetores globais de armazenamento de segmentos e pontos =================================

segment *seg;
int cur_seg;
int num_segs;
point *init_x; // Usado pra fazer a Range (pontos ordenados em x)
point *init_y; // Usado pra fazer a Range (pontos ordenados em y)
segment *queries; // usa a mesma estrutura de segmento, mas interpretada como retângulo

int *seg_x; // Usado pra fazer a Segment (segmentos ordenados em x)
int seg_x_array_size=0;
int *seg_y; // Usado pra fazer a Segment (segmentos ordenados em y)
int seg_y_array_size=0;
int *val_x; // Usado pra fazer a Segment (valores possíveis de x)
int *val_y; // Usado pra fazer a Segment (valores possíveis de y)
int *last_report; // Guardará o último retângulo no qual um segmento foi reportado.

int val_x_alloc_memory=0;
int val_x_array_size=0;
int val_y_alloc_memory=0;
int val_y_array_size;

// ATENÇÃO PARA O INCLUDE MAIS PORCO DA DÉCADA
#include "redblack.c"

r_tree *t_x=NULL;
r_tree *t_y=NULL;

// Declaração de funções principais =================================

// RANGE
x_node *build_x_top_down(point *p_x, point *p_y, int n); // VAI CONSTRUIR AS ÁRVORES X DO RANGE
y_node *build_y_bottom_up(point *a, int n); // VAI CONSTRUIR AS ÁRVORES Y DO RANGE

void ordena_x(int ini, int fim); // ORDENAÇÃO INICIAL DO VETOR X (move os elementos do vetor Y junto com ele)
void ordena_y(int ini, int fim); // ORDENAÇÃO INICIAL DO VETOR Y (preserva os índices "arr_x" obtidos no ordena_x)

void query_2d(x_node *x, int xi, int xf, int yi, int yf); // QUERY 2D DA RANGE TODA
void query_1d(y_node *y, int xi, int xf, int yi, int yf); // QUERY 1D PRAS ARVRINHA CANÔNICA
void reporta_y(y_node *y, int xi, int xf, int yi, int yf); // REPORTA A ÁRVRE INTEIRA RECURSIVAMENTE

// SEGMENTOS
s_node *build_seg_tree(int vals); // CONSTRÓI UMA ÁRVORE DE SEGMENTOS VAZIA COM 2*vals-1 FOLHAS

void ordena_val_x(int ini, int fim); // ORDENAÇÃO INICIAL DOS VALORES DE X
void ordena_val_y(int ini, int fim); // ORDENAÇÃO INICIAL DOS VALORES DE Y
void ordena_seg_x(); // ORDENAÇÕES DOS SEGMENTOS DE ACORDO COM A ORDEM DESCRITA LÁ NO TEXTO
void ordena_seg_y();
void insert_segment(s_node *tree, int s, interval i); // INSERE UM SEGMENTO DE ÍNDICE s (no vetor seg) NA ÁRVORE tree, ENTRE OS EXTREMOS DO INTERVALO i.
void varredura_x();
void varredura_y();

void query_node(s_node *s, int xi, int xf, int yi, int yf, int e, interval in); // BUSCA RECURSIVA, NODO A NODO
void query_seg(s_node *s, int xi, int xf, int yi, int yf, int e); // BUSCA NO VETOR DE SEGMENTOS ENCABEÇADO POR s A ARESTA DO RETÂNGULO [xi,xf]*[yi,yf] DE ÍNDICE e.

// ÍNDICES DAS ARESTAS:
// 0 - direita, 1 - superior, 2 - esquerda, 3 - inferior.

// Declaração de pequenas funções auxiliares =======
void troca_ponto(point *p,int a, int b); // troca os pontos de índice a e b do vetor de pontos p
int in_rect(int xp, int yp, int xi, int xf, int yi, int yf); // Só pra verificar se o ponto (xp,yp) tá no retângulo (xi,xf,yi,yf)
void reporta(int qual);
void unificar(); // TORNA ÚNICAS AS ENTRADAS DOS VETORES DE VALORES X E Y
int binary_search(int x, int *v, int size); // BUSCA BINÁRIA DE INTEIRO x NO VETOR v
int compara(segment s, segment t, int coord); // Compara os segmentos s e t de acordo com as ordens horizontais e verticais
															 // se coord==0: retorna 0 se os segmentos intersectam, 1 se o segmento s está à direita do segmento t, -1 caso contrário
															 // se coord==1: retorna 0 se os segmentos intersectam, 1 se o segmento s está acima do segmento t, -1 caso contrário
															 // IMPORTANTE: o segmento t vai sempre ser paralelo a um eixo.

// Declaração de funções de debug ====

void scream_x(x_node *x,int level); // FUNÇÃO DE DEBUG: imprime recursivamente a árvore construída pelo build_x_top_down
void scream_y(y_node *y,int level,int x_level); // FUNÇÃO DE DEBUG: imprime recursivamente a árvore construída pelo build_y_bottom_up
void scream_vals(); // FUNÇÃO DE DEBUG: imprime os vetores de possíveis valores de x e y
void scream_seg_tree(s_node *s, int level); // FUNÇÃO DE DEBUG: imprime a árvore de segmentos do ponteiro s.

// ####### MAIN #######

int main(){
	// LEITURA DA ENTRADA
	int num_boxes;
	int aux;
	scanf("%d %d",&num_segs,&num_boxes);

	// ALOCAÇÕES INICIAIS
	seg=(segment *)malloc(num_segs*sizeof(segment));
	queries=(segment *)malloc(num_boxes*sizeof(segment));
	init_x=(point *)malloc(num_segs*2*sizeof(point));
	init_y=(point *)malloc(num_segs*2*sizeof(point));
	val_x_alloc_memory=MIN_SIZE;
	val_y_alloc_memory=MIN_SIZE;
	val_x=(int *)malloc(val_x_alloc_memory*sizeof(int));
	val_y=(int *)malloc(val_y_alloc_memory*sizeof(int));
	if(!seg || !init_x || !init_y || !val_x || !val_y){
		fprintf(stderr,"Falha na alocação de memória. Tem memória suficiente mesmo?\n");
		return -1;
	}

	int i;
	last_report=(int *)malloc(num_segs*sizeof(int));
	for(i=0;i<num_segs;i++){
		scanf("%d %d %d %d",&seg[i].xi,&seg[i].xf,&seg[i].yi,&seg[i].yf);
		last_report[i]=-1;

		/* lista os dois pointos */
		init_x[2*i].s=i;
		init_x[2*i].x=seg[i].xi;
		init_x[2*i].y=seg[i].yi;
		init_y[2*i].s=i;
		init_y[2*i].x=seg[i].xi;
		init_y[2*i].y=seg[i].yi;
		init_x[(2*i)+1].s=i;
		init_x[(2*i)+1].x=seg[i].xf;
		init_x[(2*i)+1].y=seg[i].yf;
		init_y[(2*i)+1].s=i;
		init_y[(2*i)+1].x=seg[i].xf;
		init_y[(2*i)+1].y=seg[i].yf;

		/* adicionar no vetor de valores */
		if(val_x_alloc_memory <= val_x_array_size+3){
			val_x_alloc_memory+=15;
			val_x=(int *)realloc(val_x,val_x_alloc_memory*sizeof(int));
			if(!val_x){
				fprintf(stderr,"Falha na alocação de memória. Tem memória suficiente mesmo?\n");
				return -1;
			}
		}
		if(val_y_alloc_memory <= val_y_array_size+3){
			val_y_alloc_memory+=15;
			val_y=(int *)realloc(val_y,val_y_alloc_memory*sizeof(int));
			if(!val_y){
				fprintf(stderr,"Falha na alocação de memória. Tem memória suficiente mesmo?\n");
				return -1;
			}
		}
		val_x[val_x_array_size]=seg[i].xi;
		val_x_array_size++;
		val_x[val_x_array_size]=seg[i].xf;
		val_x_array_size++;		
		val_y[val_y_array_size]=seg[i].yi;
		val_y_array_size++;
		val_y[val_y_array_size]=seg[i].yf;
		val_y_array_size++;
	}
	// LEITURA DOS retângulos
	for(i=0;i<num_boxes;i++){ scanf("%d %d %d %d",&queries[i].xi,&queries[i].xf,&queries[i].yi,&queries[i].yf); 
		if(queries[i].xi > queries[i].xf){
			aux=queries[i].xi;
			queries[i].xi=queries[i].xf;
			queries[i].xf=aux;
		}
		if(queries[i].yi > queries[i].yf){
			aux=queries[i].yi;
			queries[i].yi=queries[i].yf;
			queries[i].yf=aux;
		}
	}

	// Ordenações (quicksort, ambas O(n log n))
	ordena_x(0,(2*num_segs)-1);
	for(i=0;i<2*num_segs;i++) init_y[i].arr_x=i;
	ordena_y(0,(2*num_segs)-1);
	ordena_val_x(0,val_x_array_size-1);
	ordena_val_y(0,val_y_array_size-1);
	unificar();
	x_node *range;
	range=build_x_top_down(init_x,init_y,2*num_segs); // Finalmente constrói a range (também O(n log n))
	s_node *arv_x;
	s_node *arv_y;
	arv_x=build_seg_tree(val_x_array_size);
	arv_y=build_seg_tree(val_y_array_size);
	interval in;

	// VAMOS CRIAR seg_x E seg_y
	
	v_x=(vertex **)malloc(num_segs*sizeof(vertex *));
	v_y=(vertex **)malloc(num_segs*sizeof(vertex *));
	seg_x=(int *)malloc(num_segs*sizeof(int));
	seg_y=(int *)malloc(num_segs*sizeof(int));
	varredura_x();
	varredura_y();
	for(i=0;i<num_segs;i++){	// INSERIR SEGMENTOS NA ÁRVORE X
		in.ini=binary_search(seg[seg_y[i]].xi,val_x,val_x_array_size); // isolar o intervalo no qual o tio vai
		in.fim=binary_search(seg[seg_y[i]].xf,val_x,val_x_array_size);
		if(in.ini>in.fim){
			aux=in.fim;
			in.fim=in.ini;
			in.ini=aux;
		}
		insert_segment(arv_x,seg_y[i],in);
	}
	//scream_seg_tree(arv_x,0);
	for(i=0;i<num_segs;i++){	// INSERIR SEGMENTOS NA ÁRVORE Y
		in.ini=binary_search(seg[seg_x[i]].yi,val_y,val_y_array_size); // isolar o intervalo no qual o tio vai
		in.fim=binary_search(seg[seg_x[i]].yf,val_y,val_y_array_size);

		if(in.ini>in.fim){
			aux=in.fim;
			in.fim=in.ini;
			in.ini=aux;
		}
		insert_segment(arv_y,seg_x[i],in);
	}
	//scream_seg_tree(arv_y,0);

	int e;
	for(i=0;i<num_boxes;i++){ // YAY FOR BUSCAS
		cur_seg=i;
		query_2d(range,queries[i].xi,queries[i].xf,queries[i].yi,queries[i].yf);
	
		// BUSCA NA ÁRVORE DE SEGMENTOS ---
		for(e=0;e<4;e++){ // 0 e 2 - busca na x, 1 e 3 - busca na y.
			if(e%2==0){
				query_seg(arv_x,queries[i].xi,queries[i].xf,queries[i].yi,queries[i].yf,e);
			}else{
				query_seg(arv_y,queries[i].xi,queries[i].xf,queries[i].yi,queries[i].yf,e);
			}
		}
		// Fim das BUSCA TUDO
		conta_pulos=0;
		printf("\n");
	}
	
	return 0;
}

// Código das funções principais ================================

// GERA A ÁRVORE Y DADO UM VETOR ORDENADO POR Y (A altura é garantidamente limitada superiormente por teto de log n)
y_node *build_y_bottom_up(point *a, int n){
	y_node **next;
	y_node **current;
	y_node **aux;
	current=(y_node **)malloc(n*sizeof(y_node *));
	next=(y_node **)malloc(n*sizeof(y_node *));
	int i,size_of_current,size_of_next=0;
	size_of_current=n;
	// CONSTRUÇÃO INICIAL DO VETOR DE Y's (linear no tamanho da entrada)
	for(i=0;i<n;i++){
		current[i]=malloc(sizeof(y_node));
		current[i]->p.x=a[i].x;
		current[i]->p.y=a[i].y;
		current[i]->p.s=a[i].s;
		current[i]->p.arr_x=a[i].arr_x;
		current[i]->left=NULL;
		current[i]->right=NULL;
		current[i]->parent=NULL;
	}
	while(size_of_current != 1){// A CADA NOVA ITERAÇÃO desse loop, o tamanho do vetor "current" vai ser no máximo o teto da metade do tamanho anterior.
		for(i=0;i<size_of_current;i++){ // CADA SEGUNDO CARA A PARTIR DO 0 VAI SER FOLHA DO NÍVEL ATUAL, A NÃO SER QUE A LISTA ACABE EM UM MÚLTIPLO DE QUATRO
			if(i%2==0 && (i<size_of_current-1 || i%4 !=0)){ // VAI SER "FOLHA" PARA O NÍVEL ATUAL
				if(i%4==0){ // LEFT
					current[i+1]->left=current[i];
					current[i]->parent=current[i+1];
				}else{ // RIGHT
					current[i-1]->right=current[i];
					current[i]->parent=current[i-1];
				}				
			}else{ // A EXCEÇÃO NA QUAL A LISTA ACABA EM MÚLTIPLO DE 4 VAI SER COMPUTADA SÓ LÁ PRA CIMA
				next[size_of_next]=current[i];
				size_of_next++;
			}
		}
		aux=current;
		current=next;
		next=aux;
		size_of_current=size_of_next;
		size_of_next=0;
	} // O WHILE TEM TEMPO LINEAR NO TAMANHO DA ENTRADA TAMBÉM: O(n+teto(n/2)+teto(n/4)...)
	return current[0];
}

// CONSTRUÇÃO TOP DOWN DA ÁRVORE (o tempo gasto em cada nodo é linear sobre o tamanho do conjunto canônico, 
//                                então dá que, como a soma de todos os conjuntos canônicos vai ter tamanho 
//                                O(n log n), o algoritmo também terá)
x_node *build_x_top_down(point *p_x, point *p_y, int n){
	x_node *out;
	point *left_x;
	point *left_y;
	point *right_x;
	point *right_y;
	out=(x_node *)malloc(sizeof(x_node));
	if(out == NULL){
		fprintf(stderr,"Não deu pra alocar o nodo da árvore x. Vish. \n");
		exit(-1);
	}
	out->y_tree=build_y_bottom_up(p_y,n); // CONSTRUÇÃO DA ÁRVORE DO CONJUNTO CANÔNICO (O(n))
	out->p.x=p_x[n/2].x;
	out->p.y=p_x[n/2].y;
	out->p.s=p_x[n/2].s;
	out->p.arr_x=p_x[n/2].arr_x;
	int i,ind_left=0,ind_right=0;
	if(out->y_tree->left == NULL && out->y_tree->right == NULL){ // SE FOR FOLHA
		out->left=NULL;
		out->right=NULL;
	}else{ // VAI SEPARAR EM 2 CONJUNTOS: um de 0 a n/2-1, outro de n/2+1 a n-1
		left_x=(point *)malloc((n/2)*sizeof(point));
		left_y=(point *)malloc((n/2)*sizeof(point));
		right_x=(point *)malloc((n-1-n/2)*sizeof(point));
		right_y=(point *)malloc((n-1-n/2)*sizeof(point)); // Pode parecer redundante fazer n-(n/2), mas no caso de número ímpar, vai ser necessário
		if(left_x == NULL || left_y == NULL || right_x == NULL || right_y == NULL){
			fprintf(stderr,"Não deu pra alocar os vetores de separação. Vish. \n");
			exit(-1);
		}
		for(i=0;i<n/2;i++){ 
			left_x[i].x=p_x[i].x;
			left_x[i].y=p_x[i].y;
			left_x[i].s=p_x[i].s;
		} // Split do X é intuitivo
		for(i=n/2+1;i<n;i++){ 
			right_x[i-(n/2+1)].x=p_x[i].x;
			right_x[i-(n/2+1)].y=p_x[i].y;
			right_x[i-(n/2+1)].s=p_x[i].s;
		} // Né?
		for(i=0;i<n;i++){	// Split do Y nem tanto
			if(p_y[i].arr_x<n/2){
				left_y[ind_left].x=p_y[i].x;
				left_y[ind_left].y=p_y[i].y;
				left_y[ind_left].s=p_y[i].s;
				left_y[ind_left].arr_x=p_y[i].arr_x;
				ind_left++;
			}else if(p_y[i].arr_x>n/2){
				right_y[ind_right].x=p_y[i].x;
				right_y[ind_right].y=p_y[i].y;
				right_y[ind_right].s=p_y[i].s;
				right_y[ind_right].arr_x=p_y[i].arr_x-(n/2+1);
				ind_right++;
			}
		} // MAS É LINEAR, então tá valendo
		// CHAMAR RECURSIVAMENTE, ATÉ QUE ENFIM
		if(ind_left){ 
			out->left=build_x_top_down(left_x,left_y,n/2);
			out->left->parent=out;
		}else{
			out->left=NULL;
		}
		if(ind_right){ 
			out->right=build_x_top_down(right_x,right_y,n-1-n/2);
			out->right->parent=out;
		}else{
			out->right=NULL;
		}
	}
	return out; // VOU SÓ TENTAR COMPILAR HOJE, OS TESTES VÃO ESPERAR
					// JÁ TESTEI, TÁ TUDO OK
}

void ordena_x(int ini, int fim){ // ORDENAÇÃO QUICKSORT DE X (leva y junto pra poder marcar o x_arr no final)
	if(ini==fim-1){
		if(init_x[fim].x < init_x[ini].x || (init_x[fim].x==init_x[ini].x && seg[init_x[fim].s].xi==init_x[fim].x && seg[init_x[fim].s].yi==init_x[fim].y)){
			troca_ponto(init_x,ini,fim);
			troca_ponto(init_y,ini,fim);
		}
		return;
	}
	if(ini>=fim) return;
	int pivo;
	pivo=(fim+ini)/2;
	if(pivo>fim) pivo=fim;
	if(pivo<ini) pivo=ini;
	troca_ponto(init_x,pivo,ini);
	troca_ponto(init_y,pivo,ini);
	int i;
	pivo=ini;
	for(i=ini+1;i<=fim;i++){
		if(init_x[i].x<init_x[pivo].x || (init_x[i].x==init_x[pivo].x && seg[init_x[i].s].xi==init_x[i].x && seg[init_x[i].s].yi==init_x[i].y)){
			troca_ponto(init_x,i,pivo);
			troca_ponto(init_y,i,pivo);
			pivo++;
			troca_ponto(init_x,i,pivo);
			troca_ponto(init_y,i,pivo);
		}
	}
	ordena_x(ini,pivo-1);
	ordena_x(pivo+1,fim);
}

void ordena_y(int ini, int fim){ // ORDENAÇÃO QUICKSORT DE Y (já com os valores x_arr fixados)
	if(ini==fim-1){
		if(init_y[fim].y < init_y[ini].y || (init_y[fim].y==init_y[ini].y && seg[init_y[fim].s].xi==init_y[fim].x && seg[init_y[fim].s].yi==init_y[fim].y)){
			troca_ponto(init_y,ini,fim);
		}
		return;
	}
	if(ini>=fim) return;
	int pivo;
	pivo=(fim+ini)/2;
	if(pivo>fim) pivo=fim;
	if(pivo<ini) pivo=ini;
	troca_ponto(init_y,pivo,ini);
	int i;
	pivo=ini;
	for(i=ini+1;i<=fim;i++){
		if(init_y[i].y<init_y[pivo].y || (init_y[i].y==init_y[pivo].y && seg[init_y[i].s].xi==init_y[i].x && seg[init_y[i].s].yi==init_y[i].y)){
			troca_ponto(init_y,i,pivo);
			pivo++;
			troca_ponto(init_y,i,pivo);
		}
	}
	ordena_y(ini,pivo-1);
	ordena_y(pivo+1,fim);
}

void ordena_val_x(int ini, int fim){ // EU CRIEI FUNÇÕES SEPARADAS PRA ORDENAR CADA VETOR PORQUE EU POSSO
	int aux;
	if(ini==fim-1){
		if(val_x[ini] > val_x[fim]){
			aux=val_x[fim];
			val_x[fim]=val_x[ini];
			val_x[ini]=aux;
		}
		return;
	}
	if(ini>=fim) return;
	int pivo;
	pivo=(fim+ini)/2;
	aux=val_x[ini];
	val_x[ini]=val_x[pivo];
	val_x[pivo]=aux;
	pivo=ini;
	int i;
	for(i=ini+1;i<=fim;i++){
		if(val_x[i]<val_x[pivo]){
			aux=val_x[i];
			val_x[i]=val_x[pivo];
			val_x[pivo]=aux;
			pivo++;
			aux=val_x[i];
			val_x[i]=val_x[pivo];
			val_x[pivo]=aux;
		}
	}
	ordena_val_x(ini,pivo-1);
	ordena_val_x(pivo+1,fim);
}

void ordena_val_y(int ini, int fim){ // EU CRIEI FUNÇÕES SEPARADAS PRA ORDENAR CADA VETOR PORQUE EU POSSO
	int aux;
	if(ini==fim-1){
		if(val_y[ini] > val_y[fim]){
			aux=val_y[fim];
			val_y[fim]=val_y[ini];
			val_y[ini]=aux;
		}
		return;
	}
	if(ini>=fim) return;
	int pivo;
	pivo=(fim+ini)/2;
	aux=val_y[ini];
	val_y[ini]=val_y[pivo];
	val_y[pivo]=aux;
	pivo=ini;
	int i;
	for(i=ini+1;i<=fim;i++){
		if(val_y[i]<val_y[pivo]){
			aux=val_y[i];
			val_y[i]=val_y[pivo];
			val_y[pivo]=aux;
			pivo++;
			aux=val_y[i];
			val_y[i]=val_y[pivo];
			val_y[pivo]=aux;
		}
	}
	ordena_val_y(ini,pivo-1);
	ordena_val_y(pivo+1,fim);
} // E ACHO QUE VAI PESAR NA NOTA, MAS FAZER O QUE

void query_2d(x_node *x, int xi, int xf, int yi, int yf){ // BUSCA NA RANGE GRANDONA E PÁ SEI QUE LÁ
	x_node *v;
	v=x;
	int not_found_flag=0; // MARCA SE NÃO ACHAR UM VSPLIT
	// ACHAR O VSPLIT
	while(v->p.x > xf || v->p.x < xi){
		if(v->p.x > xf){
			if(v->left != NULL){
				v=v->left;
			}else{
				not_found_flag=1;
				break;
			}
		}
		if(v->p.x < xi){
			if(v->right != NULL){
				v=v->right;
			}else{
				not_found_flag=1;
				break;
			}
		}
	}
	if(not_found_flag){ // NÃO TEM VSPLIT
		return;
	}
	if(v->left == NULL && v->right == NULL){ // É FOLHA
		if(in_rect(v->p.x,v->p.y,xi,xf,yi,yf)){
			// VERIFICA SE É O CARA INICIAL DO SEGMENTO
			if(v->p.x == seg[v->p.s].xi && v->p.y == seg[v->p.s].yi) reporta(v->p.s+1);
			// SE não for, VERIFICA o que? Se o que é tá no retângulo
			if(!in_rect(seg[v->p.s].xi,seg[v->p.s].yi,xi,xf,yi,yf)) reporta(v->p.s+1);
		}
		return;
	}
	// NÃO É FOLHA: checa se reporta v de qualquer modo
	if(in_rect(v->p.x,v->p.y,xi,xf,yi,yf)){
		// VERIFICA SE É O CARA INICIAL DO SEGMENTO
		if(v->p.x == seg[v->p.s].xi && v->p.y == seg[v->p.s].yi) reporta(v->p.s+1);
		// SE não for, VERIFICA o que? Se o que é tá no retângulo
		if(!in_rect(seg[v->p.s].xi,seg[v->p.s].yi,xi,xf,yi,yf)) reporta(v->p.s+1);
	}

	// ESCANEIA A PARTE ESQUERDA
	x_node *vf;
	vf=v->left;
	int dont_check=0;
	if(vf != NULL){
	while(vf->left != NULL || vf->right != NULL){
		if(vf->p.x >= xi){
			// VERIFICA SE É PRA REPORTAR
			if(in_rect(vf->p.x,vf->p.y,xi,xf,yi,yf)){
				if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
				if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
			}
			if(vf->right != NULL) query_1d(vf->right->y_tree,xi,xf,yi,yf); // INVESTIGA a árvore canônica direita
			if(vf->left != NULL){ 
				vf = vf->left; // VAI PRA ESQUERDA SE PUDER, CASO CONTRÁRIO VAI À MERDA
			}else{
				dont_check=1;
				break;
			}
		}else{
			if(vf->right != NULL){ // FOI LONGE DEMAIS, TEM QUE VOLTAR
				vf=vf->right;
			}else{ // VISH
				break;
			}
		}
	}
	// POR FIM, VERIFICA SE É PRA REPORTAR A FOLHA
		if(dont_check==0 && in_rect(vf->p.x,vf->p.y,xi,xf,yi,yf)){
			if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
			if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
		}
	}

	// ESCANEIA A PARTE DIREITA
	vf=v->right;
	dont_check=0;
	if(vf != NULL){
	while(vf->left != NULL || vf->right != NULL){
		if(vf->p.x <= xf){
			// VERIFICA SE É PRA REPORTAR
			if(in_rect(vf->p.x,vf->p.y,xi,xf,yi,yf)){
				if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
				if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
			}
			if(vf->left != NULL) query_1d(vf->left->y_tree,xi,xf,yi,yf); // INVESTIGA a árvore canônica esquerda
			if(vf->right != NULL){ 
				vf = vf->right; // VAI PRA DIREITA SE PUDER, CASO CONTRÁRIO VAI À MERDA
			}else{
				dont_check=1;
				break;
			}
		}else{
			if(vf->left != NULL){ // FOI LONGE DEMAIS, TEM QUE VOLTAR
				vf=vf->left;
			}else{ // VISH
				break;
			}
		}
	}
	// POR FIM, VERIFICA SE É PRA REPORTAR A FOLHA
	if(dont_check==0 && in_rect(vf->p.x,vf->p.y,xi,xf,yi,yf)){
		if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
		if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
	}
	}

}

void query_1d(y_node *y, int xi, int xf, int yi, int yf){ // EITA POARR
	// PRIMEIRO ACHA O VSPLIT
	y_node *v;
	v=y;
	int not_found_flag=0; // MARCA SE NÃO ACHAR UM VSPLIT
	// ACHAR O VSPLIT
	while(v->p.y > yf || v->p.y < yi){
		if(v->p.y > yf){
			if(v->left != NULL){
				v=v->left;
			}else{
				not_found_flag=1;
				break;
			}
		}
		if(v->p.y < yi){
			if(v->right != NULL){
				v=v->right;
			}else{
				not_found_flag=1;
				break;
			}
		}
	}
	if(not_found_flag){ // NÃO TEM VSPLIT
		return;
	}
	if(v->left == NULL && v->right == NULL){
		if(v->p.y <= yf && v->p.y >= yi){
			// VERIFICA SE É O CARA INICIAL DO SEGMENTO
			if(v->p.x == seg[v->p.s].xi && v->p.y == seg[v->p.s].yi) reporta(v->p.s+1);
			// SE não for, VERIFICA o que? Se o que é tá no retângulo
			if(!in_rect(seg[v->p.s].xi,seg[v->p.s].yi,xi,xf,yi,yf)) reporta(v->p.s+1);
		}
		return;
	}
	// NÃO É FOLHA: checa se reporta v de qualquer modo
	if(v->p.y <= yf && v->p.y >= yi){
		// VERIFICA SE É O CARA INICIAL DO SEGMENTO
		if(v->p.x == seg[v->p.s].xi && v->p.y == seg[v->p.s].yi) reporta(v->p.s+1);
		// SE não for, VERIFICA o que? Se o que é tá no retângulo
		if(!in_rect(seg[v->p.s].xi,seg[v->p.s].yi,xi,xf,yi,yf)) reporta(v->p.s+1);
	}
	
	// ESCANEAR PARA A ESQUERDA
	y_node *vf;
	vf=v->right;
	int dont_check=0;
	if(vf != NULL){
		while(vf->left != NULL || vf->right != NULL){
			if(vf->p.y >= yi){
				// VERIFICA SE É O CARA INICIAL DO SEGMENTO
				if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
				// SE não for, VERIFICA o que? Se o que é tá no retângulo
				if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
				if(vf->right != NULL) reporta_y(vf->right,xi,xf,yi,yf);
				if(vf->left != NULL){ 
					vf = vf->left; // VAI PRA ESQUERDA SE PUDER, CASO CONTRÁRIO VAI À MERDA
				}else{
					dont_check=1;
					break;
				}
			}else{
				if(vf->right != NULL){
					vf=vf->right;
				}else{
					break;
				}
			}
		}
		// POR FIM, VERIFICA SE É PRA REPORTAR A FOLHA
		if(dont_check==0 && in_rect(vf->p.x,vf->p.y,xi,xf,yi,yf)){
			if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
			if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
		}
	}

	// ESCANEAR PARA A DIREITA
	vf=v->left;
	dont_check=0;
	if(vf != NULL){
		while(vf->left != NULL || vf->right != NULL){
			if(vf->p.y <= yf){
				// VERIFICA SE É O CARA INICIAL DO SEGMENTO
				if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
				// SE não for, VERIFICA o que? Se o que é tá no retângulo
				if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
				if(vf->left != NULL) reporta_y(vf->left,xi,xf,yi,yf);
				if(vf->right != NULL){ 
					vf = vf->right; // VAI PRA DIREITA SE PUDER, CASO CONTRÁRIO VAI À MERDA
				}else{
					dont_check=1;
					break;
				}
			}else{
				if(vf->left != NULL){
					vf=vf->left;
				}else{
					break;
				}
			}
		}
		// POR FIM, VERIFICA SE É PRA REPORTAR A FOLHA
		if(dont_check==0 && in_rect(vf->p.x,vf->p.y,xi,xf,yi,yf)){
			if(vf->p.x == seg[vf->p.s].xi && vf->p.y == seg[vf->p.s].yi) reporta(vf->p.s+1);
			if(!in_rect(seg[vf->p.s].xi,seg[vf->p.s].yi,xi,xf,yi,yf)) reporta(vf->p.s+1);
		}
	}
}

void reporta_y(y_node *y, int xi, int xf, int yi, int yf){
	// Só precisa verificar nosso critério, pq esse cara tá garantidamente no retângulo
	if(y->p.x == seg[y->p.s].xi && y->p.y == seg[y->p.s].yi) reporta(y->p.s+1);
	if(!in_rect(seg[y->p.s].xi,seg[y->p.s].yi,xi,xf,yi,yf)) reporta(y->p.s+1);
	if(y->left != NULL) reporta_y(y->left, xi, xf, yi, yf);
	if(y->right != NULL) reporta_y(y->right, xi, xf, yi, yf);
}

s_node *build_seg_tree(int vals){
	// PRIMEIRA PARTE: construir a árvore de segmentos bottom up com nada contido nos nodos. O(n)
	s_node **current;
	s_node **next;
	s_node **troca;

	current=(s_node **)malloc((2*vals-1)*sizeof(s_node *));
	next=(s_node **)malloc((2*vals-1)*sizeof(s_node *));
	if(!current || !next){
		fprintf(stderr,"Falha na alocação de memória na hora de construir a árvore de segmentos. Ué?\n");
		exit(-1);
	}
	int current_count=0;
	int next_count=0;

	// CONSTRUÇÃO DOS NODOS FOLHA
	int i;
	for(i=0;i<(2*vals-1);i++){
		current[i]=(s_node *)malloc(sizeof(s_node));
		if(i%2 == 0){ // NODO DE EXTREMO
			current[i]->i.ini=i/2;
			current[i]->i.fim=i/2;
		}else{ // NODO IN BETWEEN
			current[i]->i.ini=(i-1)/2;
			current[i]->i.fim=(i+1)/2;
		}
		current[i]->lista=NULL;
		current[i]->s_array_size=0;
		current[i]->s_alloc_memory=0;
		current[i]->left=NULL;
		current[i]->right=NULL;
		current_count++;
	}

	// CONSTRUÇÃO PROCEDURAL DA ÁRVORE (eita porra)
	while(current_count > 1){
		next_count=0;
		for(i=0;i<current_count;i+=2){
			if(i!=current_count-1){ // NOVO NODO
				next[next_count]=(s_node *)malloc(sizeof(s_node));
				next[next_count]->i.ini=current[i]->i.ini;
				next[next_count]->i.fim=current[i+1]->i.fim;
				next[next_count]->lista=NULL;
				next[next_count]->s_array_size=0;
				next[next_count]->s_alloc_memory=0;
				next[next_count]->left=current[i];
				next[next_count]->right=current[i+1];
				next_count++;
			}else{ // ÚLTIMO NODO VAI PRA CIMA (se for múltiplo de 2)
				next[next_count]=current[i];
				next_count++;
			}				
		}
		// TROCA CURRENT COM NEXT
		troca=next;
		next=current;
		current=troca;
		current_count=next_count;
	}
	return current[0];
}

void insert_segment(s_node *tree, int s, interval i){
	if(tree->i.fim < i.ini || tree->i.ini > i.fim) return; // NÃO CONTINUA PROCURANDO AQUI 
	if(tree->i.ini >= i.ini && tree->i.fim <= i.fim){// É PRA INSERIR
		if(tree->s_alloc_memory==0){ // ALOCAÇÃO INICIAL
			tree->lista=(int *)malloc(MIN_SIZE*sizeof(int));
			tree->s_alloc_memory=MIN_SIZE;
			if(!tree->lista){
				fprintf(stderr, "Falha na alocação de memória. Sério? Vish.\n");
				exit(-1);
			}
		}else if(tree->s_array_size>=tree->s_alloc_memory-3){ // REALOCAR
			tree->s_alloc_memory+=15;
			tree->lista=(int *)realloc(tree->lista,tree->s_alloc_memory*sizeof(int));
			if(!tree->lista){
				fprintf(stderr, "Falha na RE-alocação de memória. Caramba...\n");
				exit(-1);
			}
		}
		tree->lista[tree->s_array_size]=s;
		tree->s_array_size++;
		return; // SE INSERIU JÁ É
	}
	if(tree->left != NULL) insert_segment(tree->left,s,i);
	if(tree->right != NULL) insert_segment(tree->right,s,i);
}

void query_seg(s_node *s, int xi, int xf, int yi, int yf, int e){
	// BUSCA O INTERVALO
	interval in;
	int buscando;
	int onde;
	// aresta 0 - verifica o intervalo para xf
	if(e==0){
		if(xf > val_x[val_x_array_size-1]) return; // Linha está pra fora do conjunto de segmentos
		buscando = xf;
		onde = 0;
	}
	// aresta 1 - verifica o intervalo para yf
	if(e==1){
		if(yf > val_y[val_y_array_size-1]) return; // Linha está pra fora do conjunto de segmentos
		buscando = yf;
		onde = 1;
	}
	// aresta 2 - verifica o intervalo para xi
	if(e==2){	
		if(xi < val_x[0]) return; // Linha está pra fora do conjunto de segmentos
		buscando = xi;
		onde = 0;
	}
	// aresta 3 - verifica o intervalo pra yi
	if(e==3){
		if(yi < val_y[0]) return; // Linha está pra fora do conjunto de segmentos
		buscando = yi;
		onde = 1;
	}

	int index;
	if(onde == 1){
		index=binary_search(buscando,val_y,val_y_array_size);
		if(buscando < val_y[index] && index > 0){
			in.ini=index-1;
			in.fim=index;
		}else	if(buscando > val_y[index] && index < val_y_array_size-1){
			in.ini=index;
			in.fim=index+1;
		}else{
			in.ini=index;
			in.fim=index;
		}
	}else{
		index=binary_search(buscando,val_x,val_x_array_size);
		if(buscando < val_x[index] && index > 0){
			in.ini=index-1;
			in.fim=index;
		}else	if(buscando > val_x[index] && index < val_x_array_size-1){
			in.ini=index;
			in.fim=index+1;
		}else{
			in.ini=index;
			in.fim=index;
		}
	}
	query_node(s,xi,xf,yi,yf,e,in);
}

void query_node(s_node *s, int xi, int xf, int yi, int yf, int e, interval in){
	if(s->s_array_size>0){
		segment aresta;
		if(e==0){
			aresta.xi=xf;
			aresta.xf=xf;
			aresta.yi=yi;
			aresta.yf=yf;
		}
		if(e==1){
			aresta.xi=xi;
			aresta.xf=xf;
			aresta.yi=yf;
			aresta.yf=yf;
		}
		if(e==2){
			aresta.xi=xi;
			aresta.xf=xi;
			aresta.yi=yi;
			aresta.yf=yf;
		}
		if(e==3){
			aresta.xi=xi;
			aresta.xf=xf;
			aresta.yi=yi;
			aresta.yf=yi;
		}
		// PRIMEIRO: faz a busca no vetor do nodo atual: busca binária pelo começo só.
		// LEMBRAR: função in_rect JÁ existe.
		int a,b,m;
		int num_iter;
		a=0;
		b=s->s_array_size-1; 
		m=(a+b)/2;
		if(compara(aresta,seg[s->lista[b]],1-(e%2))!=1 && compara(aresta,seg[s->lista[a]],1-(e%2))!=-1){// buscar começo
			while(a<=b && m>=0 && m<s->s_array_size){
				m=(a+b)/2;
				if(compara(aresta,seg[s->lista[m]],1-(e%2))==-1 || 
				(compara(aresta,seg[s->lista[m]],1-(e%2))==0 && 
				m-1>=0 && 
				compara(aresta,seg[s->lista[m-1]],1-(e%2))==0)){// aresta tá pra trás
					b=m-1;
				}
				if(compara(aresta,seg[s->lista[m]],1-(e%2))==1){// aresta tá pra fentre
					a=m+1;
				}
				if(compara(aresta,seg[s->lista[m]],1-(e%2))==0 && (m==0 || compara(aresta,seg[s->lista[m-1]],1-(e%2))==1)){
					break; // ACHOU ÇAPOHA
				}
			} // SE tiver algum cara que dá certo, ele vai ser encontrado
			num_iter=0;
			while(m<s->s_array_size && compara(aresta,seg[s->lista[m]],1-(e%2))==0){
				reporta(s->lista[m]+1);
				m++;
				num_iter++;
			}
		}// SE não entrar nesse if, é porque não tem NINGUÉM mesmo.
	}
	

	// DEPOIS: migra para o próximo sujeito
	if(s->left != NULL && s->left->i.ini <= in.ini && s->left->i.fim >=in.fim)
		query_node(s->left,xi,xf,yi,yf,e,in);
	if(s->right != NULL && s->right->i.ini <= in.ini && s->right->i.fim >=in.fim)
		query_node(s->right,xi,xf,yi,yf,e,in);
}

void varredura_x(){ // VAI VARRER AS PORRA TUDO EM ORDEM DE y E VAI ORDENAR O seg_x
	int i,j;
	r_node **rs;
	rs=(r_node **)malloc(num_segs*sizeof(r_node *));
	for(j=0;j<num_segs;j++) rs[j]=NULL;
	for(i=0;i<num_segs;i++)	v_x[i]=create_vertex(i);
	t_x=(r_tree *)malloc(sizeof(r_tree));
	t_x->root=NULL;
	t_x->size=0;
	for(i=0;i<2*num_segs;i++){
		j=init_y[i].s;
		if(rs[j]==NULL){ // É PRA ADD
			rs[j]=create_node(j);
			insert_r_node(rs[j],t_x,0);
		}else{ // É PRA REMOVER
			remove_r_node(rs[j],t_x);
		}
	}
	ordering_x();
}

void varredura_y(){ // VAI VARRER AS PORRA TUDO EM ORDEM DE x E VAI ORDENAR O seg_y
	int i,j;
	r_node **rs;
	rs=(r_node **)malloc(num_segs*sizeof(r_node *));
	for(j=0;j<num_segs;j++) rs[j]=NULL;
	for(i=0;i<num_segs;i++)	v_y[i]=create_vertex(i);
	t_y=(r_tree *)malloc(sizeof(r_tree));
	t_y->root=NULL;
	t_y->size=0;
	for(i=0;i<2*num_segs;i++){
		j=init_x[i].s;
		if(rs[j]==NULL){ // É PRA ADD
			rs[j]=create_node(j);
			insert_r_node(rs[j],t_y,1);
		}else{ // É PRA REMOVER
			remove_r_node(rs[j],t_y);
		}
		
	}
	ordering_y();
}

// Código das funções pequenininhas =======

void unificar(){
	//COMEÇA UNIFICANDO O X
	int *unif;
	int i,j;
	j=0;
	unif=(int *)malloc(val_x_array_size*sizeof(int));
	if(unif == NULL){
		fprintf(stderr,"Falhou a alocação do unif, vish.\n");
		exit(-1);
	}
	unif[0]=val_x[0];
	for(i=1;i<val_x_array_size;i++){
		if(val_x[i] != unif[j]){
			j++;
			unif[j]=val_x[i];
		}
	}
	free(val_x);
	val_x=unif;
	val_x_array_size=j+1;

	//DEPOIS O Y
	j=0;
	unif=(int *)malloc(val_y_array_size*sizeof(int));
	if(unif == NULL){
		fprintf(stderr,"Falhou a alocação do unif, vish.\n");
		exit(-1);
	}
	unif[0]=val_y[0];
	for(i=1;i<val_y_array_size;i++){
		if(val_y[i] != unif[j]){
			j++;
			unif[j]=val_y[i];
		}
	}
	free(val_y);
	val_y=unif;
	val_y_array_size=j+1;
}

void troca_ponto(point *p, int a, int b){
	point aux;
	aux.s=p[a].s;
	aux.x=p[a].x;
	aux.y=p[a].y;
	aux.arr_x=p[a].arr_x;
	p[a].s=p[b].s;
	p[a].x=p[b].x;
	p[a].y=p[b].y;
	p[a].arr_x=p[b].arr_x;
	p[b].s=aux.s;
	p[b].x=aux.x;
	p[b].y=aux.y;
	p[b].arr_x=aux.arr_x;
}

int in_rect(int xp, int yp, int xi, int xf, int yi, int yf){
	if(xp >= xi &&
		xp <= xf &&
		yp >= yi &&
		yp <= yf) return 1;
	return 0;
}

void reporta(int qual){
	if(last_report[qual-1]!=cur_seg){
		if(conta_pulos==0){ printf("%d",qual);
		}else{ printf(" %d",qual);}
		conta_pulos++;
		last_report[qual-1]=cur_seg;
	}
}

int binary_search(int x, int *v, int size){
	int i,f,m;
	i=0;
	f=size-1;
	while(i<=f && i<size && f>=0){
		m=(i+f)/2;
		if(v[m]>x){
			f=m-1;
		}else if(v[m]<x){
			i=m+1;
		}else{
			return m;
		}
	}
	return m;
}// 1 - se o elemento estiver no vetor, será encontrado.
 // 2 - se o elemento não estiver no vetor, o valor retornado será ou o último valor menor, ou o primeiro valor maior, que está contido no vetor.

int compara(segment s, segment t, int coord){ // SE coord==0, UM DOS CARAS TEM QUE SER PARALELO AO EIXO X, O MESMO PRA coord==1 E Y
	int sxmin,txmin,sxmax,txmax,symin,tymin,symax,tymax;	
	if(coord==0){ // COMPARAÇÃO EM X
		if(s.yi==s.yf && t.yi==t.yf){ // DOIS PARALELOS AO EIXO X
			if(s.xi<=s.xf){
				sxmin=s.xi;
				sxmax=s.xf;
			}else{
				sxmin=s.xf;
				sxmax=s.xi;
			}
			if(t.xi<=t.xf){
				txmin=t.xi;
				txmax=t.xf;
			}else{
				txmin=t.xf;
				txmax=t.xi;
			}
			if((sxmin<=txmin && sxmax>=txmin) || (txmin <=sxmin && txmax>=sxmin)){ // INTERSECTAM
				return 0;
			}
			if(sxmin<=txmin) return -1;
			return 1;
		}
		if(s.yi==s.yf) return compara(t,s,0)*(-1); // SÓ O s É.
		// CONSTRUIR x=f(y) DE s
		double a,b;
		if(s.xi==s.xf){
			a=0;
			b=s.xi;
		}else if(s.yi!=0){
			b=((double)s.xi*((double)s.yf/(double)s.yi)-(double)s.xf)/(((double)s.yf/(double)s.yi)-1);
			a=((double)s.xi-(double)b)/(double)s.yi;
		}else{
			b=((double)s.xf*((double)s.yi/(double)s.yf)-(double)s.xi)/(((double)s.yi/(double)s.yf)-1);
			a=((double)s.xf-(double)b)/(double)s.yf;
		}
		if(a*(double)t.yi+b>=(double)t.xi && a*(double)t.yf+b>=(double)t.xf) return 1;
		if(a*(double)t.yi+b<=(double)t.xi && a*(double)t.yf+b<=(double)t.xf) return -1;
		return 0;



	}else{ // COMPARAÇÃO EM Y
		if(s.xi==s.xf && t.xi==t.xf){ // DOIS PARALELOS AO EIXO Y
			if(s.yi<=s.yf){
				symin=s.yi;
				symax=s.yf;
			}else{
				symin=s.yf;
				symax=s.yi;
			}
			if(t.yi<=t.yf){
				tymin=t.yi;
				tymax=t.yf;
			}else{
				tymin=t.yf;
				tymax=t.yi;
			}
			if((symin<=tymin && symax>=tymin) || (tymin <=symin && tymax>=symin)){ // INTERSECTAM
				return 0;
			}
			if(symin<=tymin) return -1;
			return 1;
		}
		if(s.xi==s.xf) return compara(t,s,1)*(-1); // SÓ O s É.
		// CONSTRUIR y=f(x) DE s
		double a,b;
		if(s.yi==s.yf){
			a=0;
			b=s.yi;
		}else if(s.xi!=0){
			b=((double)s.yi*((double)s.xf/(double)s.xi)-(double)s.yf)/(((double)s.xf/(double)s.xi)-1);
			a=((double)s.yi-(double)b)/(double)s.xi;
		}else{
			b=((double)s.yf*((double)s.xi/(double)s.xf)-(double)s.yi)/(((double)s.xi/(double)s.xf)-1);
			a=((double)s.yf-(double)b)/(double)s.xf;
		}
		if(a*(double)t.xi+b>=(double)t.yi && a*(double)t.xf+b>=(double)t.yf) return 1;
		if(a*(double)t.xi+b<=(double)t.yi && a*(double)t.xf+b<=(double)t.yf) return -1;
		return 0;
	}
}

// Código das funções de debug ====

// FUNÇÃO DEBUG: imprime a árvore X da range com nível separado por "="
void scream_x(x_node *x,int level){
	int i;
	for(i=0;i<level;i++) printf("=");
	printf(" Xnode, x: %d, y: %d, s: %d\n",x->p.x,x->p.y,x->p.s);
	for(i=0;i<level;i++) printf("=");
	printf(" Y_tree:\n"); // IMPRIMIR A ÁRVORE Y PENDURADA (do conjunto canônico)
	scream_y(x->y_tree,0,level);
	for(i=0;i<level;i++) printf("=");
	printf(" Xnode left:\n");
	if(x->left != NULL) scream_x(x->left,level+1);
	for(i=0;i<level;i++) printf("=");
	printf(" Xnode right:\n");
	if(x->right != NULL) scream_x(x->right,level+1);
}

// FUNÇÃO DEBUG: imprime a árvore Y da range com indentação.
void scream_y(y_node *y,int level,int x_level){
	int i;
	for(i=0;i<x_level;i++) printf("=");
	for(i=0;i<level;i++) printf("\t");
	printf("x: %d, y: %d, s: %d, arr_x: %d\n",y->p.x,y->p.y,y->p.s,y->p.arr_x);
	for(i=0;i<x_level;i++) printf("=");
	for(i=0;i<level;i++) printf("\t");
	printf("Left: \n");
	if(y->left != NULL)
		scream_y(y->left,level+1,x_level);
	for(i=0;i<x_level;i++) printf("=");
	for(i=0;i<level;i++) printf("\t");
	printf("Right: \n");
	if(y->right != NULL)
		scream_y(y->right,level+1,x_level);
}

void scream_seg_tree(s_node *s, int level){ // GRITAR A ÁRVORE DE SEGMENTOS
	int i;
	for(i=0;i<level;i++) printf("=");
	printf(" ini: %d, fim: %d\n", s->i.ini, s->i.fim);
	if(s->lista != NULL){ // VAI IMPRIMIR A LISTA
		for(i=0;i<level;i++) printf("=");
		printf(" Lista: ");
		for(i=0;i<s->s_array_size;i++){
			printf("%d ",s->lista[i]);
		}
		printf("\n");
	}
	for(i=0;i<level;i++) printf("=");
	printf(" Left:\n");
	if(s->left != NULL) scream_seg_tree(s->left,level+1);
	for(i=0;i<level;i++) printf("=");
	printf(" Right:\n");
	if(s->right != NULL) scream_seg_tree(s->right,level+1);
}

void scream_vals(){
	int i;
	for(i=0;i<val_x_array_size;i++) printf("%d ",val_x[i]);
	printf("\n");
	for(i=0;i<val_y_array_size;i++) printf("%d ",val_y[i]);
	printf("\n");
	printf("VALORES X E Y ACIMA\n");
}


