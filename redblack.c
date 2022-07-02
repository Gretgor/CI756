#define RED 0
#define BLACK 1
// ARQUIVO CRIADO SEPARADO PRA EVITAR ENCHER DEMAIS AQUELE PRIMEIRO. VOU DAR INCLUDE DE MODO PORCO MESMO, QUE SE DANE

typedef struct r_node{
	int entry; // ÍNDICE DO VETOR DE SEGMENTOS
	int color; // 0 = vermelho, 1 = preto
	struct r_node *parent;
	struct r_node *left;
	struct r_node *right;
}r_node;

typedef struct{
	r_node *root;
	int size;
}r_tree;


struct vertex; // ODEIO TER QUE FAZER ISSO, MAS FAZER O QUE

typedef struct edge_pointer{
	struct vertex *in;
	struct vertex *out;
	struct edge_pointer *prev;
	struct edge_pointer *next;
	struct edge_pointer *corr; // O CORRESPONDENTE NA LISTA DO OUTRO VÉRTICE
}edge_pointer;

typedef struct vertex{
	int entry; // ÍNDICE DO VETOR DE SEGMENTOS
	int in_count;
	int out_count;
	edge_pointer *first_incoming;
	edge_pointer *first_outcoming;
}vertex;

vertex **v_x; // VÉRTICES PRA CRIAR A ORDEM
vertex **v_y; // VÃO TER O TAMANHO DO NÚMERO DE SEGMENTOS, E O ÍNDICE DELES CORRESPONDERÁ AO SEGMENTO

// ===================================================================================================
// COISARADA DO GRAFO PRA ORDENAÇÃO TOPOLÓGICA =======================================================
// ===================================================================================================

vertex *create_vertex(int entry){ // CRIAÇÃO DO VÉRTICE, SIMPLÃO
	vertex *v;
	v=(vertex *)malloc(sizeof(vertex));
	v->entry=entry;
	v->in_count=0;
	v->out_count=0;
	v->first_incoming=NULL;
	v->first_outcoming=NULL;
	return v;
}

edge_pointer *a;
edge_pointer *b; // PARA DEBUG

int rofl=0;

void add_edge(vertex *v, vertex *w){ // ADICIONAR ARESTA, BEM
	edge_pointer *first_v;
	edge_pointer *first_w;
	first_v=v->first_outcoming;
	first_w=w->first_incoming;
	edge_pointer *new_v;
	edge_pointer *new_w;
	new_v=(edge_pointer *)malloc(sizeof(edge_pointer));
	new_w=(edge_pointer *)malloc(sizeof(edge_pointer));
	new_v->out=v;
	new_v->in=w;
	new_v->corr=new_w;
	new_w->out=v;
	new_w->in=w;
	new_w->corr=new_v;
	new_v->prev=NULL;
	new_v->next=first_v;
	if(first_v!=NULL){
		first_v->prev=new_v;
	}
	new_w->prev=NULL;
	new_w->next=first_w;
	if(first_w!=NULL){
		first_w->prev=new_w;
	}
	v->first_outcoming=new_v;
	w->first_incoming=new_w;
	v->out_count++;
	w->in_count++;
}

void remove_edge(edge_pointer *e){
	edge_pointer *corr;
	corr=e->corr;

	// REMOÇÃO DAS PORRA TUDO
	if(e->next != NULL){
		e->next->prev=e->prev;
	}
	if(e->prev != NULL){
		e->prev->next=e->next;
	}
	if(e==e->in->first_incoming){
		e->in->first_incoming=e->next;
	}
	if(e==e->out->first_outcoming){
		e->out->first_outcoming=e->next;
	}

	if(corr->next != NULL){
		corr->next->prev=corr->prev;
	}
	if(corr->prev != NULL){
		corr->prev->next=corr->next;
	}
	if(corr==corr->in->first_incoming){
		corr->in->first_incoming=corr->next;
	}
	if(corr==corr->out->first_outcoming){
		corr->out->first_outcoming=corr->next;
	}
	e->in->in_count--;
	e->out->out_count--;

	free(e);
	free(corr);
}

void ordering_x(){ // KAAAAAAAAAAAAAAAAAAAHN (Star Trek feelings)
	vertex **no_incoming_edges;
	vertex *v;
	vertex *w;
	no_incoming_edges=(vertex **)malloc(num_segs*sizeof(vertex *));
	int ni_array_size=0;
	int i;
	for(i=0;i<num_segs;i++){
		if(v_x[i]->in_count==0){
			no_incoming_edges[ni_array_size]=v_x[i];
			ni_array_size++;
		}
	}
	while(ni_array_size){
		v=no_incoming_edges[ni_array_size-1];
		ni_array_size--;
		seg_x[seg_x_array_size]=v->entry;
		seg_x_array_size++;
		while(v->out_count > 0){
			w=v->first_outcoming->in;
			remove_edge(v->first_outcoming);
			if(w->in_count == 0){
				no_incoming_edges[ni_array_size]=w;
				ni_array_size++;
			}
		}
	}
}

void ordering_y(){ // KAAAAAAAAAAAAAAAAAAAHN (Star Trek feelings)
	vertex **no_incoming_edges;
	vertex *v;
	vertex *w;
	no_incoming_edges=(vertex **)malloc(num_segs*sizeof(vertex *));
	int ni_array_size=0;
	int i;
	for(i=0;i<num_segs;i++){
		if(v_y[i]->in_count==0){
			no_incoming_edges[ni_array_size]=v_y[i];
			ni_array_size++;
		}
	}
	while(ni_array_size){
		v=no_incoming_edges[ni_array_size-1];
		ni_array_size--;
		seg_y[seg_y_array_size]=v->entry;
		seg_y_array_size++;
		while(v->out_count > 0){
			w=v->first_outcoming->in;
			remove_edge(v->first_outcoming);
			if(w->in_count == 0){
				no_incoming_edges[ni_array_size]=w;
				ni_array_size++;
			}
		}
	}
}

// ===================================================================================================
// COISARADA DA ÁRVORE RUBRO-NEGRA ===================================================================
// ===================================================================================================

int watafaqui=0;

int is_above(int s, int t){ // VERIFICA SE O SEGMENTO DE ÍNDICE s TÁ EM CIMA DO SEGMENTO DE ÍNDICE t: DADO QUE A GENTE SABE QUE UM TÁ EM CIMA DO OUTRO.
	// E SE s FOR PARALELO AO EIXO Y, E t TAMBÉM? VERIFICA SÓ PELO y MENOR!
	int ys,yt;
	if(seg[s].xi == seg[s].xf && seg[t].xi == seg[t].xf){
		if(seg[s].yi < seg[s].yf){ 
			ys=seg[s].yi;
		}else{
			ys=seg[s].yf;
		}
		if(seg[t].yi < seg[t].yf){ 
			yt=seg[t].yi;
		}else{
			yt=seg[t].yf;
		}
		if(ys>=yt){
			return 1;
		}else{
			return 0;
		}
	}

	// E SE s FOR E t NÃO?
	if(seg[s].xi == seg[s].xf){
		if(!is_above(t,s)) return 1;
		return 0;
	}

	// E SE s NÃO FOR? Aí vem a parte legal. 
	
	//PARTIU CALCULAR A EQUAÇÃO y=f(x) PRA s!
	double as,bs;
	double coefs[2][2];
	double results[2];
	if(seg[s].xi!=0){
		coefs[0][0]=(double)seg[s].xi;
		coefs[0][1]=1;
		results[0]=(double)seg[s].yi;
		coefs[1][0]=(double)seg[s].xf;
		coefs[1][1]=1;
		results[1]=(double)seg[s].yf;
	}else{
		coefs[0][0]=(double)seg[s].xf;
		coefs[0][1]=1;
		results[0]=(double)seg[s].yf;
		coefs[1][0]=(double)seg[s].xi;
		coefs[1][1]=1;
		results[1]=(double)seg[s].yi;
	}
	double multiplier=coefs[1][0]/coefs[0][0];
	coefs[1][0]=0;
	coefs[1][1]=coefs[1][1]-(coefs[0][1]*multiplier);
	results[1]=results[1]-(results[0]*multiplier);
	bs=results[1]/coefs[1][1];
	as=(results[0]-bs)/coefs[0][0];

	// SE t FOR vertical: só verificar se ambos seus pontos tão pra BAIXO
	if(seg[t].xi==seg[t].xf){
		if((as*(double)seg[t].xi+bs)>=(double)seg[t].yi && (as*(double)seg[t].xf+bs)>=(double)seg[t].yf) return 1;
		return 0;
	}

	//CASO CONTRÁRIO
	//PARTIU CALCULAR A EQUAÇÃO y=f(x) PRA t!
	double at,bt;
	if(seg[t].xi!=0){
		coefs[0][0]=(double)seg[t].xi;
		coefs[0][1]=1;
		results[0]=(double)seg[t].yi;
		coefs[1][0]=(double)seg[t].xf;
		coefs[1][1]=1;
		results[1]=(double)seg[t].yf;
	}else{
		coefs[0][0]=(double)seg[t].xf;
		coefs[0][1]=1;
		results[0]=(double)seg[t].yf;
		coefs[1][0]=(double)seg[t].xi;
		coefs[1][1]=1;
		results[1]=(double)seg[t].yi;
	}
	multiplier=coefs[1][0]/coefs[0][0];
	coefs[1][0]=0;
	coefs[1][1]=coefs[1][1]-(coefs[0][1]*multiplier);
	results[1]=results[1]-(results[0]*multiplier);
	bt=results[1]/coefs[1][1];
	at=(results[0]-bt)/coefs[0][0];

	// AGORA carcula com os ponto de s e t: um necessariamente vai ser tal que a equação tá toda pra cima (ou pra baixo) do outro.
	if( ((as*(double)seg[t].xi+bs)>=(double)seg[t].yi && (as*(double)seg[t].xf+bs)>=(double)seg[t].yf) ||
		((at*(double)seg[s].xi+bt)<=(double)seg[s].yi && (at*(double)seg[s].xf+bt)<=(double)seg[s].yf) ) return 1;
	return 0;
}

// SOCORRO JESUS

int is_right(int s, int t){ // VERIFICA SE O SEGMENTO DE ÍNDICE s TÁ A DIREITA DO SEGMENTO DE ÍNDICE t: DADO QUE A GENTE SABE QUE UM TÁ A DIREITA DO OUTRO.
	// E SE s FOR PARALELO AO EIXO X, E t TAMBÉM? VERIFICA SÓ PELO X MENOR!
	int xs,xt;
	if(seg[s].yi == seg[s].yf && seg[t].yi == seg[t].yf){
		if(seg[s].xi < seg[s].xf){ 
			xs=seg[s].xi;
		}else{
			xs=seg[s].xf;
		}
		if(seg[t].xi < seg[t].xf){ 
			xt=seg[t].xi;
		}else{
			xt=seg[t].xf;
		}
		if(xs>=xt){
			return 1;
		}else{
			return 0;
		}
	}

	// E SE s FOR E t NÃO?
	if(seg[s].yi == seg[s].yf){
		if(!is_right(t,s)) return 1;
		return 0;
	}

	// E SE s NÃO FOR? Aí vem a parte legal. 
	
	//PARTIU CALCULAR A EQUAÇÃO x=f(y) PRA s!
	double as,bs;
	double coefs[2][2];
	double results[2];
	if(seg[s].yi!=0){
		coefs[0][0]=(double)seg[s].yi;
		coefs[0][1]=1;
		results[0]=(double)seg[s].xi;
		coefs[1][0]=(double)seg[s].yf;
		coefs[1][1]=1;
		results[1]=(double)seg[s].xf;
	}else{
		coefs[0][0]=(double)seg[s].yf;
		coefs[0][1]=1;
		results[0]=(double)seg[s].xf;
		coefs[1][0]=(double)seg[s].yi;
		coefs[1][1]=1;
		results[1]=(double)seg[s].xi;
	}
	double multiplier=coefs[1][0]/coefs[0][0];
	coefs[1][0]=0;
	coefs[1][1]=coefs[1][1]-(coefs[0][1]*multiplier);
	results[1]=results[1]-(results[0]*multiplier);
	bs=results[1]/coefs[1][1];
	as=(results[0]-bs)/coefs[0][0];

	// SE t FOR horizontal: só verificar se ambos seus pontos tão pra ESQUERDA
	if(seg[t].yi==seg[t].yf){
		if((as*(double)seg[t].yi+bs)>=(double)seg[t].xi && (as*(double)seg[t].yf+bs)>=(double)seg[t].xf) return 1;
		return 0;
	}

	//CASO CONTRÁRIO
	//PARTIU CALCULAR A EQUAÇÃO x=f(y) PRA t!
	double at,bt;
	if(seg[t].yi!=0){
		coefs[0][0]=(double)seg[t].yi;
		coefs[0][1]=1;
		results[0]=(double)seg[t].xi;
		coefs[1][0]=(double)seg[t].yf;
		coefs[1][1]=1;
		results[1]=(double)seg[t].xf;
	}else{
		coefs[0][0]=(double)seg[t].yf;
		coefs[0][1]=1;
		results[0]=(double)seg[t].xf;
		coefs[1][0]=(double)seg[t].yi;
		coefs[1][1]=1;
		results[1]=(double)seg[t].xi;
	}
	multiplier=coefs[1][0]/coefs[0][0];
	coefs[1][0]=0;
	coefs[1][1]=coefs[1][1]-(coefs[0][1]*multiplier);
	results[1]=results[1]-(results[0]*multiplier);
	bt=results[1]/coefs[1][1];
	at=(results[0]-bt)/coefs[0][0];

	// AGORA carcula com os ponto de s e t: um necessariamente vai ser tal que a equação tá toda pra esquerda (ou pra direita) do outro.
	if( ((as*(double)seg[t].yi+bs)>=(double)seg[t].xi && (as*(double)seg[t].yf+bs)>=(double)seg[t].xf) ||
		((at*(double)seg[s].yi+bt)<=(double)seg[s].xi && (at*(double)seg[s].yf+bt)<=(double)seg[s].xf) ) return 1;
	return 0;
}

r_node *create_node(int entry){
	r_node *r;
	r=malloc(sizeof(r_node));
	r->color=RED;
	r->entry=entry;
	r->parent=NULL;
	r->left=NULL;
	r->right=NULL;
	return r;
}

// NODO VOVÔ
r_node *avo(r_node *r){
	if(r->parent != NULL) return r->parent->parent;
	return NULL;
}

// NODO TITIO DO PAVÊ
r_node *tio(r_node *r){
	r_node *t;
	t=avo(r);
	if(t != NULL){
		if(r->parent == t->left) return t->right;
		return t->left;
	}
	return NULL;
}

// NODO IRMÃO SEM NOÇÃO
r_node *irmao(r_node *r){
	if(r->parent==NULL) return NULL;
	if(r==r->parent->left) return r->parent->right;
	return r->parent->left;
}

void verifica_caso1(r_node *r, r_tree *t);
void verifica_caso2(r_node *r, r_tree *t);
void verifica_caso3(r_node *r, r_tree *t);
void verifica_caso4(r_node *r, r_tree *t);
void verifica_caso5(r_node *r, r_tree *t);

r_node *verificar;

r_node *busca_antecessor(r_node *r);
r_node *busca_sucessor(r_node *r);
// INSERIR NODO
void insert_r_node(r_node *r, r_tree *t, int coord){
	t->size++;
	if(t->size==1){
		t->root=r;
		r->color=BLACK;
		return;
	}
	r_node *q;
	r_node *last;
	last=NULL;
	r->color=RED;
	// PRIMEIRO ENCONTRA A CASA DO NOVO NODO
	if(r->entry == 4) watafaqui=1;
	q=t->root;
	int acabou_menor=0;
	while(q != NULL){
		last=q;
		if((coord==0 && is_right(r->entry,q->entry)) || (coord==1 && is_above(r->entry,q->entry))){
			q=q->right;
			acabou_menor=0;
		}else{
			q=q->left;
			acabou_menor=1;
		}
	}
	r->parent=last;
	if(acabou_menor){
		last->left=r;
	}else{
		last->right=r;
	}
	
	// ACABOU A INSERÇÃO, HORA DE CORRIGIR AS ESTRUTURA TUDO
	verifica_caso1(r,t);

	// AGORA ENCONTRAR ANTECESSOR E SUCESSOR PRA FAZER O GRAFO (calma, Fabricio, não grita)
	r_node *antecessor;
	r_node *sucessor;
	antecessor=busca_antecessor(r);
	sucessor=busca_sucessor(r);
	
	if(coord==0){
		if(antecessor!=NULL){
			add_edge(v_x[antecessor->entry],v_x[r->entry]);
		}
		if(sucessor!=NULL){
			add_edge(v_x[r->entry],v_x[sucessor->entry]);
		}
	}else{
		if(antecessor!=NULL){
			add_edge(v_y[antecessor->entry],v_y[r->entry]);
		}
		if(sucessor!=NULL){
			add_edge(v_y[r->entry],v_y[sucessor->entry]);
		}
	}
	
}

// TANTO A BUSCA POR ANTECESSOR QUANTO POR SUCESSOR VÃO SER O(log n)
// AFINAL, A ÁRVORE VAI TER ALTURA O(log n), E A BUSCA VAI NO MÁXIMO ATÉ A RAIZ OU ATÉ UMA FOLHA

r_node *busca_antecessor(r_node *r){
	r_node *q;
	if(r->left!=NULL){ // VAI BUSCAR NA SUBARVORE ESQUERDA
		q=r->left;
		while(q->right != NULL){
			q=q->right;
		}
		return q;
	}
	
	// VAI BUSCAR O PRIMEIRO ANCESTRAL À ESQUERDA
	q=r->parent;
	r_node *p;
	p=r;
	while(q!=NULL && p==q->left){
		p=q;
		q=q->parent;
	}
	if(q==NULL) return NULL; // NÃO TEM ANTECESSOR
	return q;
}

r_node *busca_sucessor(r_node *r){
	r_node *q;
	if(r->right!=NULL){ // VAI BUSCAR NA SUBARVORE DIREITA
		q=r->right;
		while(q->left != NULL){
			q=q->left;
		}
		return q;
	}
	
	// VAI BUSCAR O PRIMEIRO ANCESTRAL À DIREITA
	q=r->parent;
	r_node *p;
	p=r;
	while(q!=NULL && p==q->right){
		p=q;
		q=q->parent;
	}
	if(q==NULL) return NULL; // NÃO TEM ANTECESSOR
	return q;
}

void remove_one_child(r_node *r, r_tree *t);
void considerar_casos(r_node *filho,r_tree *t, r_node *parent, r_node *s, int lado_pai);

void remove_r_node(r_node *r,r_tree *t){
	t->size--;
	if(r->left == NULL || r->right == NULL){ 
		
		remove_one_child(r,t);
		
		return;
	}
	r_node *q;
	// SE TIVER MAIS DE UM FILHO, COPIA O VALOR DO ANTECESSOR E O REMOVE
	q=busca_antecessor(r);
	r->entry=q->entry;
	remove_one_child(q,t);
}

void remove_one_child(r_node *r, r_tree *t){
		
	if(r->left == NULL && r->right == NULL && r->color==RED){ // DÁ PRA EXTRAIR O CARA DE BOA.
		if(r->parent!= NULL && r->parent->left==r) r->parent->left=NULL;
		if(r->parent!= NULL && r->parent->right==r) r->parent->right=NULL;


		//free(r);

		return;
	}
		
	r_node *filho;
	if(r->left == NULL){
		filho=r->right;
	}else{
		filho=r->left;
	}

	// PARTIU REFATORAR A P*** TODA

	r_node *parent;
	parent=r->parent;

	int lado_pai=0; // registrar se o filho é à esquerda ou à direita.
	if(parent != NULL){
		if(r==parent->left){ 
			lado_pai=0;
		}else{ 
			lado_pai=1;
		}
	}

	// Tá, começa substituindo os nodo tudo.
	if(parent!= NULL){		
		if(r==parent->left){ 
			parent->left=filho;
		}else{ 
			parent->right=filho;
		}
	}
	if(filho != NULL) filho->parent=parent;
	r_node *s;
	s=irmao(r);

	if(r->color == RED){ // OUTRO CASO INTUITIVO: FILHO PRETO E PAI VERMELHO.
      //if(filho != NULL) filho->parent=parent; NUNCA VAI ACONTECER
		if(r==t->root) t->root=filho;
		//free(r);
		return;
	}

	// MAIS UM CASO ÓBVIO: FILHO VERMELHO, PAI PRETO
	if(r->color==BLACK && filho != NULL && filho->color==RED){
		filho->color=BLACK;
		if(r==t->root){ t->root=filho;
			filho->color=BLACK;
		}
		//free(r);
		return;
	}
	//free(r);

	// A PARTIR DAQUI VAI TUDO VIRAR OUTRA FUNÇÃO: CASO EM QUE FILHO E PAI SÃO PRETOS
	considerar_casos(filho,t,parent,s,lado_pai);
}

void rotate_left(r_node *r, r_tree *t);
void rotate_right(r_node *r, r_tree *t);

// OS CASOS DA INSERÇÃO NA RED-BLACK ================================

void verifica_caso1(r_node *r, r_tree *t){
	if(r->parent == NULL){
		t->root=r;
		r->color=BLACK;
		return;
	}
	verifica_caso2(r,t);
}

void verifica_caso2(r_node *r, r_tree *t){
	if(r->parent->color==BLACK) return;
	verifica_caso3(r,t);
}

void verifica_caso3(r_node *r, r_tree *t){
	r_node *ti;
	ti=tio(r);
	r_node *u;
	if(ti!=NULL && ti->color==RED){
		r->parent->color=BLACK;
		ti->color=BLACK;
		u=avo(r);
		u->color=RED;
		verifica_caso1(u,t);
		return;
	}
	verifica_caso4(r,t);
}

void verifica_caso4(r_node *r, r_tree *t){
	r_node *a;
	r_node *p;
	r_node *q;	
	a=avo(r);
	if(r==r->parent->right && r->parent==a->left){ // VISH, PARTIU ROTACIONAR
		rotate_left(r->parent,t);
		r=r->left;
	}else if(r==r->parent->left && r->parent==a->right){
		rotate_right(r->parent,t);
		r=r->right;
	}
	verifica_caso5(r,t);
}

void verifica_caso5(r_node *r, r_tree *t){

	r_node *a;
	a=avo(r);
	r_node *p;
	r_node *q;	
	r->parent->color=BLACK;
	a->color=RED;
	if(r==r->parent->left){
		rotate_right(a,t);
	}else{
		rotate_left(a,t);
	}
}

void rotate_left(r_node *r, r_tree *t){
	r_node *b;
	r_node *a;
	r_node *q=NULL;
	b=r->right;
	a=r->parent;
	if(a!= NULL){
		if(a->left == r){
			a->left=b;
		}else{
			a->right=b;
		}
	}
	if(b != NULL) q=b->left;
	if(b != NULL) b->left=r;
	r->right=q;
	if(b != NULL) b->parent=a;
	r->parent=b;
	if(a==NULL){
		t->root=b;
		if(b!=NULL) b->color=BLACK;
	}
	if(q!=NULL) q->parent=r;
	
}

void rotate_right(r_node *r, r_tree *t){
	r_node *b;
	r_node *a;
	r_node *q=NULL;
	b=r->left;
	a=r->parent;
	if(a != NULL){
		if(a->left == r){
			a->left=b;
		}else{
			a->right=b;
		}
	}
	if (b != NULL) q=b->right;
	if(b != NULL) b->right=r;
	r->left=q;
	if(b != NULL) b->parent=a;
	r->parent=b;
	if(a==NULL){
		t->root=b;
		if(b!=NULL) b->color=BLACK;
	}
	if(q!=NULL) q->parent=r;
	
}

// CASOS DE REMOÇÃO NA REDBLACK ===========================================



void considerar_casos(r_node *filho,r_tree *t, r_node *parent, r_node *s, int lado_pai){

	
	
	// OUTRO CASO ÓBVIO: filho É A NOVA RAIZ
	if(parent == NULL){
		t->root=filho;
		if(filho != NULL){
			filho->parent=NULL;
			filho->color=BLACK;
		}
		return;
	}

	

	// A PARTIR DESSE PONTO, SABEMOS QUE parent != NULL

	// A PARTIR DESSE PONTO, O PAI ATUAL DE filho É parent, MESMO QUE FILHO SEJA NULL
	// lado_pai também registra de que lado de parent o filho está.
	
	// CASO DIFÍCIL: FILHO PRETO, PAI PRETO.
	// A PARTIR DESSE PONTO, TEMOS QUE s É O IRBÃO DE filho
	// CASO 2: s é vermelho
	// NESSE CASO, sabemos que s tem seus dois filhos não nulos.
	
	if(s!= NULL && s->color == RED){
		s->color=BLACK;
		parent->color=RED;
		if(lado_pai==0){
			rotate_left(parent,t);
		}else{
			rotate_right(parent,t);
		}
		if(lado_pai==0){ // REENCONTRAR O MANINHO
			s=parent->right;
		}else{
			s=parent->left;
		}
	}

	

	int orientacao_chamada=0;
	if(parent->parent != NULL){
		if(parent==parent->parent->left){
			orientacao_chamada=0;
		}else{
			orientacao_chamada=1;
		}
	}

	r_node *s_chamada;
	s_chamada=irmao(parent);

	// CASO 3: parent, s e os filhos de s são pretos

	// NA VERDADE, pela propriedade 5, a gente sabe que s aqui nunca é NULL (se s fosse NULL, teríamos que antes das operações de remoção, um caminho iria de parent
	// passaria por r e chegaria nas folhas a partir de filho com no mínimo 2 nodos pretos (3 se parent for preto), e o caminho até s teria só 1 (2 se parent for preto))
	if(parent->color==BLACK && (s!=NULL && s->color==BLACK) && (s->left==NULL || s->left->color==BLACK) && (s->right==NULL || s->right->color==BLACK)){
		s->color=RED;
		considerar_casos(parent,t,parent->parent,s_chamada,orientacao_chamada);
		return;
	}

	

	// CASO 4: s E OS FILHOS DE s SÃO PRETOS, MAS p É VERMELHO (novamente, sabemos que s!= NULL pelos comentários acima)
	if(parent->color==RED && (s!=NULL && s->color==BLACK) && (s->left==NULL || s->left->color==BLACK) && (s->right==NULL || s->right->color==BLACK)){
		s->color=RED;
		parent->color=BLACK;
		return;
	}
	

	// CASO 5: s é preto, tem uma criança vermelha e outra preta, tal que a vermelha está ao mesmo lado de s que filho está de parent.
	if(s!=NULL && s->color == BLACK){
		if(lado_pai==0 && (s->right == NULL || s->right->color==BLACK ) && (s->left != NULL && s->left->color == RED)){
			s->color=RED;
			s->left->color=BLACK;
			rotate_right(s,t);
		}
		if(lado_pai==1 && (s->left == NULL || s->left->color==BLACK ) && (s->right != NULL && s->right->color == RED)){
			s->color=RED;
			s->right->color=BLACK;
			rotate_left(s,t);
		}
		return;
	}

	

	// CASO 6: s é preto, tem uma criança vermelha e outra preta, tal que a vermelha está do lado oposto de s com relação ao qual filho está de parent.
	if(s!=NULL && s->color == BLACK){
		s->color=parent->color;
		parent->color=BLACK;
		if(lado_pai==0 && (s->right != NULL && s->right->color==RED ) && (s->left == NULL || s->left->color == BLACK)){
			s->right->color=BLACK;
			rotate_left(parent,t);
		}
		if(lado_pai==1 && (s->left != NULL && s->left->color==RED ) && (s->right == NULL || s->right->color == BLACK)){
			s->left->color=BLACK;
			rotate_right(parent,t);
		}
		return;
	}

	
}
