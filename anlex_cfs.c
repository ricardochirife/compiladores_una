#include <stdio.h>
#include <string.h>

#define MAX_PAL_RES 4
#define MAX_LEX 50

typedef enum 	//definimos los tipos de tokens
{
	TERMINADOR_PUNTOCOMA,
	TERMINADOR_ENTER,
	L_PARENTESIS,
	R_PARENTESIS,
	OP_CONDICION,						
	OP_SUMA,
	OP_MUL,
	OP_RELACIONAL,
	OP_ASIGNACION,
	L_CORCHETE,
	R_CORCHETE,
	COMA,
	DELIMITADOR_CODIGO,
	PR_IF,
	PR_TRUE,
	PR_FALSE,
	PR_ALERT,
	LITERAL_NUM,
	LITERAL_CADENA,
	ID,
	COMMENT,
	_EOF,
	ERROR_LEXICO	// Token para manejar errores lexicos
}tokenType;

typedef struct 	//definimos la estructura de un token: componente lexico (tokenType) y el lexema
{
	tokenType compLex;
	char *lexema;
}token;

token t;

static struct 	//definimos las palabras reservadas y asociamos a su componente lexico
{
	char* palabra;
	tokenType token_t;
}palabrasReservadas[MAX_PAL_RES]={{"IF",PR_IF},{"TRUE",PR_TRUE},{"FALSE",PR_FALSE},{"ALERT",PR_ALERT}};

typedef enum 	//estados del automata para identificar que expresion regular esta reconociendo
{
	INICIO,EN_ID,EN_NUMERO,EN_COMENTARIO,EN_LITERAL,FIN
}estado;

token getToken(void); 			//Devulve el siguiente token del fuente
tokenType buscarReservadas(char *p); 	//Busca si una cadena es una palabra reservada o es un id
void imprimirSalida(token k); 	//Imprime el componente lexico al archivo de salida
void saltarLinea();				//Salta a la siguiente línea del fuente
token errorLexico();			//Devuelve un token de error lexico
void aMayus(char a[], char b[]);//Convierte una cadena a mayusculas. Uso para buscar las palabras reservadas

estado estadoActual;//estado actual del automata
int estado_num=1;	//estado actual del automata que reconoce numeros
int linea=1;		//numero de linea
int linea_aux=0;	//numero de linea donde comenzo un literal. Se utiliza para manejar errores en literales que contienen \n
int consumir=0;		//determina si el caracter debe ser consumido o devuelto al fuente
char c=' ';
int terminar=0;		//bandera para terminar de analizar el fuente

FILE *fuente=NULL;
FILE *salida=NULL;

int main (int argc, char* args[])
{
	if(argc > 1)
		{
			if (!(fuente=fopen(args[1],"rt")))
			{
				printf("\nanlex_cfs ERROR. Archivo no encontrado.\n");
				exit(1);
			}
			if (argc==2)
				salida=fopen("output.txt","w+");
			else if(argc==3)
				salida=fopen(args[2],"w+");
			else
			{
				printf("\nanlex_cfs ERROR. Parámetros Incorrectos.\n");
				exit(1);
			}
			while(!terminar)
			{
				t=getToken();
				imprimirSalida(t);
			}
			fclose(fuente);
			fclose(salida);
		}
		else
		{
			printf("\nanlex_cfs ERROR. Debe pasar como parámetro el path al archivo fuente.\n");
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
		c = fgetc(fuente);
		consumir=1;
		switch(estadoActual)
		{
			case INICIO:
				if(isdigit(c)) 	//si encuentra un digito, cambiamos el estado actual del automata y vamos a reconocer numeros
				{
					estadoActual=EN_NUMERO;
					estado_num=1;
				}
				else if((isalpha(c))||(c=='_'))	//si encuentra una letra o un guion bajo, reconocemos identificadores
					estadoActual=EN_ID;
				else if(c=='"')
				{
					linea_aux=linea;
					estadoActual=EN_LITERAL; //si encuentra comillas, reconocemos literales
				}
				else if(c=='#')
					estadoActual=EN_COMENTARIO;	//si encuentra numeral, reconocemos comentarios
				else if((c==' ')||(c=='\t')) 	// ignoramos espacios en blanco y tabulaciones
					continue;
				else if(c=='\n')
				{
					linea++;
					estadoActual=FIN;
					compLexActual=TERMINADOR_ENTER;
				}
				else if(c=='+')
				{
					estadoActual=FIN;
					compLexActual=OP_SUMA;
				}
				else if(c=='-')	//si encontramos un - , y el siguiente caracter es un >  consumimos los dos ya que es un token de dos caracteres
				{				//sino es un >, devolvemos el caracter al fuente
					lexemaActual[lexemaIndex++]=c;	
					estadoActual=FIN;
					if((c=fgetc(fuente))=='>')
					{
						compLexActual=DELIMITADOR_CODIGO;
					}
					else
					{
						ungetc(c,fuente);
						consumir=0;
						compLexActual=OP_SUMA;
					}
				}
				else if((c=='*')||(c=='/'))
				{
					estadoActual=FIN;
					compLexActual=OP_MUL;
				}
				else if(c=='<')	//Si viene un < y despues un = consumimos los dos, sino devolvemos el caracter
				{
					lexemaActual[lexemaIndex++]=c;
					estadoActual=FIN;
					compLexActual=OP_RELACIONAL;
					if((c=fgetc(fuente))!='=')
					{
						ungetc(c,fuente);
						consumir=0;
					}
				}
				else if(c=='>'	)	//Si viene un > y despues un = consumimos los dos, sino devolvemos el caracter
				{
					lexemaActual[lexemaIndex++]=c;
					estadoActual=FIN;
					compLexActual=OP_RELACIONAL;
					if((c=fgetc(fuente))!='=')
					{
						ungetc(c,fuente);
						consumir=0;
					}
				}
				else if(c=='=')		//Si viene un = y despues otro = consumimos los dos, sino devolvemos el caracter
				{
					lexemaActual[lexemaIndex++]=c;	
					estadoActual=FIN;
					if((c=fgetc(fuente))=='=')
					{
						compLexActual=OP_RELACIONAL;
					}
					else
					{
						ungetc(c,fuente);
						consumir=0;
						compLexActual=OP_ASIGNACION;
					}
				}
				else if(c=='!')		//Si viene un !, necesariamente el siguiente caracter debe ser un = segun la gramatica. Si no viene el = mostramos error
				{
					lexemaActual[lexemaIndex++]=c;
					if((c=fgetc(fuente))=='=')
					{
						estadoActual=FIN;
						compLexActual=OP_RELACIONAL;
					}
					else
					{
						if(c==EOF)
							printf("\n ERROR: No se esperaba fin de archivo");
						else
							printf("\n ERROR LEXICO: LÍNEA %d: Se esperaba = encontró el caracter \"%c\"",linea,c);
						saltarLinea();
						return errorLexico();
					}
					
				}
				else if(c==']')
				{
					estadoActual=FIN;
					compLexActual=R_CORCHETE;
				}
				else if(c=='[')
				{
					estadoActual=FIN;
					compLexActual=L_CORCHETE;
				}
				else if(c==',')
				{
					estadoActual=FIN;
					compLexActual=COMA;
				}
				else if(c=='(')
				{
					estadoActual=FIN;
					compLexActual=L_PARENTESIS;
				}
				else if(c==')')
				{
					estadoActual=FIN;
					compLexActual=R_PARENTESIS;
				}
				else if(c==';')
				{
					estadoActual=FIN;
					compLexActual=TERMINADOR_PUNTOCOMA;
				}
				else if(c=='?')
				{
					estadoActual=FIN;
					compLexActual=OP_CONDICION;
				}
				else if(c==EOF)								//Si encontramos fin de archivo, levantamos la bandera para que termine el analizador lexico
				{
					terminar=1;
					estadoActual=FIN;
					compLexActual=_EOF;
				}
				else
				{
					printf("\n ERROR LEXICO: LÍNEA %d: No se esperaba el caracter \"%c\" ",linea,c);
					saltarLinea();
					return errorLexico();
				}
				break;
			case EN_ID:			//reconocemos identificadores
				if((!(isalpha(c)))&&(!(isdigit(c)))&&(c!='_'))
				{
					ungetc(c,fuente);
					consumir=0;
					estadoActual=FIN;
					compLexActual=ID;			
				}
				break;
			case EN_NUMERO:
				switch(estado_num)	//reconocemos numeros
				{
					case 1:		//estado 1 pueden venir numeros, un . o una e
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
					case 2:		//estado 2 Aceptacion. Vino el punto y necesariamente debe venir un numero
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
					case 3:		//estado 3 Aceptacion. Pueden venir numeros o una e
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
					case 4:		//estado 4. Vino la e y pueden venir + - o numeros
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
					case 5:		//estado 5. Vino un + o -. Deben venir numeros
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
					case 6:		//estado 6 Aceptacion. Solo numeros
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
			case EN_COMENTARIO:	//estamos reconociendo comentarios. Ignoramos todo hasta que venga un enter o EOF
				while((c!='\n')&&(c!=EOF))
				{
					c=fgetc(fuente);
				}
				ungetc(c,fuente);
				consumir=0;
				estadoActual=FIN;
				compLexActual=COMMENT;
				break;
			case EN_LITERAL:	//estamos reconociendo literales. Seguimos hasta que vengan las comillas de cierre
				if(c=='"')
				{
					estadoActual=FIN;
					compLexActual=LITERAL_CADENA;
				}
				else if(c=='\n')
				{
					linea++;
				}
				else if(c==EOF)	//se termino el archivo antes de cerrar las comillas. error
				{
					printf("\n ERROR: LINEA %d: Se llegó al fin de archivo sin terminar un literal con ' \" '",linea_aux);
					estadoActual=FIN;
					return errorLexico();
				}
				break;
		}
		if((consumir)&&(lexemaIndex<=MAX_LEX))	//vamos almacenando los caracteres que deben se consumidos para formar el lexema
			lexemaActual[lexemaIndex++]=c;
		if(estadoActual==FIN)								
		{
			lexemaActual[lexemaIndex]='\0'; //si estado es fin agregamos el caracter de fin de cadena al lexema
			if(compLexActual==ID)			//en caso de que el token sea un ID buscamos en la lista de palabras reservadas
				compLexActual=buscarReservadas(lexemaActual);
		}
	}
	tokenActual.compLex=compLexActual;
	tokenActual.lexema=lexemaActual;
	//printf("\nlexema: %s",lexemaActual); //linea de prueba
	return tokenActual;
}

void imprimirSalida(token k) //imprime el componente lexico en el archivo de salida
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
			fputs("DELIMITADOR_CODIGO   ",salida);
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
	int i;
	char pp[strlen(p)+1];
	aMayus(p,pp);
	for(i=0;i<MAX_PAL_RES;i++)
	{
		if((strcmp(pp,palabrasReservadas[i].palabra))==0)
			return palabrasReservadas[i].token_t;
	}
	return ID;
}

void saltarLinea()
{
	while((c!='\n')&&(c!=EOF))
	{
		c=fgetc(fuente);
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
void aMayus (char a[], char b[])
{
	int i=0;
	while(a[i])
	{
		b[i]=toupper(a[i]);
		i++;
	}
	b[i]='\0';
}




