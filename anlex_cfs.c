#include <stdio.h>
#include <string.h>

#define MAX_PAL_RES 4
#define MAX_LEX 50

typedef enum 							//definimos los tipos de tokens
{
	TERMINADOR_PUNTOCOMA,		// ;
	TERMINADOR_ENTER,			// /n
	L_PARENTESIS,				// (
	R_PARENTESIS,				// )
	OP_CONDICION,				// ?
	OP_SUMA,					// + -
	OP_MUL,						// / *
	OP_RELACIONAL,				// < <= > >= != ==
	OP_ASIGNACION,				// =
	L_CORCHETE,					// [
	R_CORCHETE,					// ]
	COMA,						// ,
	DELIMITADOR_CODIGO,			// ->
	PR_IF,						// if IF
	PR_TRUE,					// true TRUE
	PR_FALSE,					// false FALSE
	PR_ALERT,					// alert ALERT
	LITERAL_NUM,				// Números con parte decimal y exponentes opcionales
	LITERAL_CADENA,				// Cadenas ".*"
	ID,							// Identificadores
	COMMENT,					// Comentarios #...
	_EOF,						// Fin de Archivo
	ERROR_LEXICO				// Token para manejar errores lexicos
}tokenType;

typedef struct 							//definimos la estructura de un token: componente lexico (tokenType) y el lexema
{
	tokenType compLex;
	char *lexema;
}token;

token t;

static struct 							//definimos las palabras reservadas y asociamos a su componente lexico
{
	char* palabra;
	tokenType token_t;
}palabrasReservadas[MAX_PAL_RES]={{"IF",PR_IF},{"TRUE",PR_TRUE},{"FALSE",PR_FALSE},{"ALERT",PR_ALERT}};

typedef enum 							//estados del automata
{
	INICIO,EN_ID,EN_NUMERO,EN_COMENTARIO,EN_LITERAL,FIN
}estado;

token getToken(void); 					//Devulve el siguiente token del fuente
tokenType buscarReservadas(char *p); 	//Busca si una cadena es una palabra reservada o es un id
void imprimirSalida(token k); 			//Imprime el archivo de salida
void saltarLinea();
token errorLexico();
estado estadoActual;
int estado_num=1;
int linea=1;
int consumir=0;
char c=' ';

FILE *fuente=NULL;
FILE *salida=NULL;

int main (int argc, char* args[])
{
	if(argc > 1)
		{
			if (!(fuente=fopen(args[1],"rt")))
			{
				printf("Archivo no encontrado.\n");
				exit(1);
			}
			salida=fopen("output.txt","w+");
			while(!feof(fuente))
			{
				t=getToken();
				imprimirSalida(t);
			}
			fclose(fuente);
			fclose(salida);
		}else{
			printf("Debe pasar como parametro el path al archivo fuente.\n");
			exit(1);
		}
		printf("\n");
		return 0;
}

token getToken()
{
	token tokenActual;
	char lexemaActual[MAX_LEX+1]="";
	int lexemaIndex=0;
	tokenType compLexActual;
	estadoActual=INICIO;
	while(estadoActual!=FIN)
	{
		c = getc(fuente);
		consumir=1;
		switch(estadoActual)
		{
			case INICIO:
				if(isdigit(c))
				{
					estadoActual=EN_NUMERO;
					estado_num=1;
				}
				else if(isalpha(c))
					estadoActual=EN_ID;
				else if(c=='"')
					estadoActual=EN_LITERAL;
				else if(c=='#')
					estadoActual=EN_COMENTARIO;
				else if((c==' ')||(c=='\t'))
					continue;
				else if(c=='\n')
				{
					linea++;
					estadoActual=FIN;
					compLexActual=TERMINADOR_ENTER;
				}
				//~ else if((c==' ')||(c=='\t'))
				//~ else if((c=='+')||(c='-'))
				//~ else if((c=='*')||(c=='/'))
				//~ else if(c=='<')
				//~ else if(c=='>')
				//~ else if(c=='=')
				//~ else if(c=='[')
				//~ else if(c==']')
				//~ else if(c==',')
				//~ else if(c=='(')
				//~ else if(c==')')
				//~ else if(c==';')
				//~ else if(c=='?')
				break;
			case EN_ID:
				if((!(isalpha(c)))&&(!(isdigit(c))))
				{
					ungetc(c,fuente);
					consumir=0;
					estadoActual=FIN;
					compLexActual=ID;			
				}
				break;
			case EN_NUMERO:
				switch(estado_num)
				{
					case 1:
						if(isdigit(c))
							estado_num=1;
						else if(c=='.')
							estado_num=2;
						else if((c=='e')||(c=='E'))
							estado_num=4;
						else
						{
							ungetc(c,fuente);
							consumir=0;
							estadoActual=FIN;
							compLexActual=LITERAL_NUM;
						}
						break;
					case 2:
						if(isdigit(c))
							estado_num=3;
						else
						{
							if(c==EOF)
								printf("\n ERROR: No se esperaba fin de archivo");
							else
								printf("\n ERROR LEXICO: LÍNEA %d: Se esperaba un número y se encontró el caracter \"%c\"",linea,c);
							saltarLinea();
							return errorLexico();
						}
						break;
					case 3:
						if(isdigit(c))
							estado_num=3;
						else if(tolower(c)=='e')
							estado_num=4;
						else
						{
							ungetc(c,fuente);
							consumir=0;
							estadoActual=FIN;
							compLexActual=LITERAL_NUM;
						}
						break;
					case 4:
						if((c=='+')||(c=='-'))
							estado_num=5;
						else if(isdigit(c))
							estado_num=6;
						else
						{
							if(c==EOF)
								printf("\n ERROR: No se esperaba fin de archivo");
							else
								printf("\n ERROR LEXICO: LÍNEA %d: Se esperaba un + o un - o un número y se encontró el caracter \"%c\"",linea,c);
							saltarLinea();
							return errorLexico();
						}
						break;
					case 5:
						if(isdigit(c))
							estado_num=6;
						else
						{
							if(c==EOF)
								printf("\n ERROR: No se esperaba fin de archivo");
							else
							printf("\n ERROR LEXICO: LÍNEA %d: Se esperaba un número y se encontró el caracter \"%c\"",linea,c);
							saltarLinea();
							return errorLexico();
						}
						break;
					case 6:
						if(isdigit(c))
							estado_num=6;
						else
						{
							ungetc(c,fuente);
							consumir=0;
							estadoActual=FIN;
							compLexActual=LITERAL_NUM;
						}
				}
				break;
			case EN_COMENTARIO:
				while(c=getc(fuente)!='\n');
				ungetc(c,fuente);
				consumir=0;
				estadoActual=FIN;
				compLexActual=COMMENT;
				break;
			case EN_LITERAL:
				while(c=getc(fuente))
				break;
		}
		if((consumir)&&(lexemaIndex<=MAX_LEX))
			lexemaActual[lexemaIndex++]=c;
		if(estadoActual==FIN)
		{
			lexemaActual[lexemaIndex]='\0'; 
			if(compLexActual==ID)
				compLexActual=buscarReservadas(lexemaActual);
		}
	}
	tokenActual.compLex=compLexActual;
	tokenActual.lexema=lexemaActual;
	//printf("\nlexema: %s",lexemaActual);
	return tokenActual;
}

void imprimirSalida(token k)
{	switch (k.compLex)
	{
		case TERMINADOR_PUNTOCOMA:
			fputs("TERMINADOR_PUNTOCOMA   ",salida);
			break;
		case TERMINADOR_ENTER:
			fputs("TERMINADOR_ENTER \n",salida);
			break;
		case L_PARENTESIS:
			fputs("L_PARENTESIS   ",salida);
			break;
		case R_PARENTESIS:
			fputs("R_PARENTESIS   ",salida);
			break;
		case OP_CONDICION:
			fputs("OP_CONDICION   ",salida);
			break;
		case OP_SUMA:
			fputs("OP_SUMA   ",salida);
			break;
		case OP_MUL:
			fputs("OP_MUL   ",salida);
			break;
		case OP_RELACIONAL:
			fputs("OP_RELACIONAL   ",salida);
			break;
		case OP_ASIGNACION:
			fputs("OP_ASIGNACION   ",salida);
			break;
		case L_CORCHETE:
			fputs("L_CORCHETE   ",salida);
			break;
		case R_CORCHETE:
			fputs("R_CORCHETE   ",salida);
			break;
		case COMA:
			fputs("COMA   ",salida);
			break;
		case DELIMITADOR_CODIGO:
			fputs("DELIMITADOR CODIGO   ",salida);
			break;
		case PR_IF:
			fputs("PR_IF   ",salida);
			break;
		case PR_TRUE:
			fputs("PR_TRUE   ",salida);
			break;
		case PR_FALSE:
			fputs("PR_FALSE   ",salida);
			break;
		case PR_ALERT:
			fputs("PR_ALERT   ",salida);
			break;
		case LITERAL_NUM:
			fputs("LITERAL_NUM   ",salida);
			break;
		case LITERAL_CADENA:
			fputs("LITERAL_CADENA   ",salida);
			break;
		case ID:
			fputs("ID   ",salida);
			break;
		case COMMENT:
			fputs("COMMENT   ",salida);
			break;
		case _EOF:
			fputs("EOF   ",salida);
			break;
		case ERROR_LEXICO:
			fputs("ERROR_LEXICO   \n",salida);
			break;
	}
}


tokenType buscarReservadas(char *p)
{
	return ID;
}

void saltarLinea()
{
	while((c!='\n')&&(c!=EOF))
	{
		c=getc(fuente);
	}
	if(c=='\n')
		linea++;
}

token errorLexico()
{
	token tk;
	tk.compLex=ERROR_LEXICO;
	tk.lexema="";
	return tk;
}




