#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include "terminalCouleur.h"
#include "saisieM.h"

#define MOVE_RIGHT (-1)
#define MOVE_LEFT 1
#define MOVE_DOWN (-1)
#define MOVE_UP 1

typedef struct{
	int n;
	int max_value;
	int free_slots;
	int *grid;
}game;

/*!
 * Alloue la grille de la variable jeu passée par adresse.
 * Initialise les cases de la grille avec des cases vides (valeurs nulles)
 * Initialise les champs n et max_value avec les valeurs passées en paramètre
 *
 * \param p : pointeur sur une partie de 2048
 * \param n : taille de la grille
 * \param max_value : valeur à atteindre pour gagner
 */
void init_game (game *p, int n, int max_value){
	p->grid = calloc(n*n, sizeof(int));
	p->n = n;
	p->max_value = max_value;
	p->free_slots = n*n;
}

/*!
 * Libère la mémoire allouée pour la grille du jeu passé par adresse.
 *
 * \param p : pointeur sur une partie de 2048
 */
void free_memory (game *p){
	free(p->grid);
}

/*!
 * Fonction retournant 1 si la case (i, j) existe, 0 sinon.
 *
 */
int valid_slot (game *p, int line, int column){
    return (line>=0 && line<(p->n) && column>=0 && column<(p->n));
}

/*!
 * Fonction retournant la valeur de la case (ligne, colonne) de la partie p,
 * ou -1 si la case n’existe pas.
 *
 * \param p : pointeur sur la partie en cours
 * \param line : entier correspondant au numéro de ligne
 * \param column : entier correspondant au numéro de colonne
 */
int get_value (game *p, int line, int column){
    return (p->grid[((p->n)*line)+column]*(valid_slot(p, line, column))-!(valid_slot(p, line, column)));
}

/*!
 * Fonction modifiant la valeur de la case (ligne, colonne) de la partie p, avec la valeur val
 *
 * \param p : pointeur sur la partie en cours
 * \param line : entier orrespondant au numéro de ligne
 * \param column : entier orrespondant au numéro de colonne
 * \param val : entier à mettre dans la case (i, j) (si elle existe)
 */
void set_value (game*p, int line, int column, int val){
    if(valid_slot(p, line, column)){
		p->grid[((p->n)*line)+column] = val;
	}
}

/*!
 * Fonction affichant une grille en couleur à l’écran.
 *
 * Le typedef enum COULEUR_TERMINAL contenu dans terminalCouleur.h nous permet de traiter chaque couleur
 * du panel comme étant un entier compris entre 0 et 7.
 * On attribue donc grâce à un log de base 2 une valeur comprise entre 0 et 7 à la couleur de la case courante,
 * couleur variant ainsi selon la puissance de 2 comprise dans la case en question.
 *
 *
 * \param p : pointeur sur la partie que l’on souhaite afficher
 */
void show (game *p){
    int line, column, curr_val;
    COULEUR_TERMINAL bg_color, fg_color, absolute_color;
	
	clear_terminal();

    for(line=0; line<(p->n); line++){
		/*
		 * Chaque ligne a (p->n) colonnes de hauteur egale a 3 pour donner de l'epaisseur. Donc chaque ligne est composee de 3 sous-lignes.
		 * curr_val : valeur de la case actuelle
		 * absolute_color : puissance de 2 correspondant à curr_val (si curr_val=8, absolute_color=3 car 8=2^3)
		 * fg_color : couleur d'ecriture (On ecrira toujours en blanc, plus simple que de changer a chaque couleur et plus lisible)
		 * bg_color : couleur de fond (On module par 7 pour ne pas avoir de case de couleur blanche)
		 */
		for(column=0; column<(p->n)*3; column++){
            fg_color = WHITE;
			curr_val = get_value(p, line, column%(p->n));
            absolute_color = (COULEUR_TERMINAL)(log2((double)(curr_val)));
            bg_color = (absolute_color%7)+(absolute_color>=7);

			// Si on est au debut d'une sous-ligne, on fait un saut de ligne et on decale de 3 tabulations pour centrer l'affichage
            if(column%(p->n)==0){
                printf("\n\t\t\t");
			}

			/*
			 * Si on est pas sur la sous-ligne du milieu on affiche seulement une chaine vide de 7 caracteres.
			 * Sinon si la valeur de la case est 0, on affiche un point sur 7 caracteres.
			 * Sinon on affiche la valeur de la case sur 7 caracteres en tenant compte de la longueur du chiffre a afficher.
			 */
            if(column<(p->n) || column>=2*(p->n)){
				color_printf(fg_color, bg_color, "       ");
			}
            else if(curr_val==0){
				color_printf(fg_color, bg_color, "   .   ");
            }
			else{
				if(curr_val<10){
                    color_printf(fg_color, bg_color, "   %d   ", curr_val);
                }
				else if(curr_val<100){
                    color_printf(fg_color, bg_color, "   %d  ", curr_val);
                }
				else if(curr_val<1000){
                    color_printf(fg_color, bg_color, "  %d  ", curr_val);
				}
            }
		}
	}
	printf("\n");
}

/*!
 * Retourne 1 si la case est vide, 0 sinon
 */
int empty_slot (game *p, int line, int column){
	return (get_value(p, line, column)==0);
}

/*!
 * Ajoute une valeur (2 ou 4 choisi aléatoirement) sur une case vide
 * (elle aussi choisie aléatoirement).
 */
void set_random_value (game *p){
	int *free_slots[p->free_slots];
	int slot, val, 
		dim = (p->n), 
		empty = 0;

	srand(time(NULL));
	
	/*
	 * On parcourt tout le plateau de jeu pour identifier les cases vides.
	 * Des qu'une case vaut 0 on stocke son adresse dans le tableau de pointeurs sur entier free_slots.
	 * Ce tableau contient donc l'adresse de chacune des cases vides a la sortie de la boucle.
	 */
	for(slot=0; slot<(dim*dim); slot++){
		if(get_value(p, slot/dim, slot%dim)==0){
			free_slots[empty++] = &(p->grid[(dim*(slot/dim))+slot%dim]);
		}
	}

	/*
	 * Si l'on a au moins une case de libre, on ajoute une valeur aleatoirement.
	 * La case a remplir est tiree au hasard parmis les indices possibles de free_slots[] et on y accede directement grace a son adresse stockee dans ce tableau.
	 * Enfin on diminue de 1 le nombre de cases vides.
	 */
	if(p->free_slots>0){
		val = ((rand()%2)+1)*2;
		slot = rand()%(p->free_slots);
		*free_slots[slot] = val;
		(p->free_slots)--;
	}
}

/*!
 * Retourne 1 si la partie est gagnée, 0 sinon.
 */
int victory (game *p){
	int slot = 0, 
		dim = p->n;
	
	do{
		if(get_value(p, slot/dim, slot%dim)==(p->max_value)){
			return 1;
		}
	}while (++slot<(dim*dim));
	
	return 0;
}

/*!
 * Retourne 1 si la partie est perdue, 0 sinon.
 */
int defeat (game*p){
    int line, column, val,
		slot = 0,
		dim = p->n, 
		limit = (dim*dim)-1;

	if(p->free_slots == 0){
		do{
			line = slot/dim, column = slot%dim, val = get_value(p, line, column);
			if(((val == get_value(p, line+1, column) && line != (dim-1)) 
				|| (val == get_value(p, line, column+1) && column != (dim-1)))
				&& val != 0){
				return 0;
			}
	    }while(++slot<limit);
	}
	return (p->free_slots==0);
}

/*!
 * Retourne 1 si la partie est terminée, 0 sinon.
 */
int game_over (game*p){
	return (victory(p) || defeat(p));
}

/*!
 * Effectue les mouvements (à gauche ou à droite) des cases d’une ligne.
 * Renvoie 1 si l’on a déplacé au moins une case, 0 sinon.
 *
 * \param p : pointeur sur un jeu
 * \param line : indice de ligne
 * \param direction : 1 pour déplacement vers la gauche et -1 pour un déplacement vers la droite
 */
int move_line (game *p, int line, int direction){
    /*
	 * start vaudra (n-1) si on va vers la droite ou 0 si on va vers la gauche. C'est la variable correspondant au départ des boucles utilisees.
	 * limit vaudra 0 si on va vers la droite ou (n-1) si on va vers la gauche. C'est la variable correspondant à la fin des boucles utilisees.
	 * On parcourt donc la ligne de gauche à droite si on veut se deplacer vers la gauche. On la parcourt de droite à gauche sinon.
	 */
	int column, other_val, curr_val,
        size = (p->n-1),
        start = size*(direction==MOVE_RIGHT),
        limit = size*(direction==MOVE_LEFT),
        move = 0,
		pack = start;
    
	/*
	 * On tasse la ligne du cote du deplacement voulu.
	 * pack ne va augmenter que lorsqu'on rencontrera une case non vide, ce qui permet de deplacer les elements non nuls dans les cases vides.
	 * On ajoute a ca le vidage des cases deplacees pour reellement tasser la ligne et ne pas dupliquer de valeurs.
	 * S'il y a deplacement, on incremente move.
	 */
	for(column=start; column!=(limit+direction); column+=direction){
		if(get_value(p, line, column)!=0){
			set_value(p, line, pack, get_value(p, line, column));
			if(pack!=column){
				set_value(p, line, column, 0);
				move++;
			}
			pack += direction;
        }
    } 

	//Cette boucle effectue la fusion des cases identiques.
    for(column=start; column!=limit; column+=direction){
        other_val = get_value(p, line, column+direction);
        curr_val = get_value(p, line, column);

		//Si deux cases ont la meme valeur et que ca n'est pas 0, on stocke la somme dans la case courante, on incremente le nombre de cases vides et move (il y a deplacement).
        if(other_val==curr_val && curr_val!=0){    
			move++;
    	    (p->free_slots)++;
            set_value(p, line, column, curr_val+other_val);

			//On decale ensuite d'un cran dans la direction voulue toute les cases non modifiees et on mets les cases d'origine de celles decalees a 0.
            for(pack=(column+direction); pack!=limit; pack+=direction){
                set_value(p, line, pack, get_value(p, line, pack+direction));
				set_value(p, line, pack+direction, 0);
            }
			//Pour mettre la derniere case de la ligne a 0 (pas pris en compte par la boucle).
			set_value(p, line, pack, 0);
        }
    }
	//Si move est different de 0, il y a eu au moins un mouvement.
    return (move!=0);
}

/*!
 * Effectue les mouvements (à gauche ou à droite) des cases sur toutes les lignes.
 * Renvoie 1 si l’on a déplacé au moins une case, 0 sinon.
 *
 * \param p : pointeur sur un jeu
 * \param direction : 1 pour déplacement vers la gauche et -1 pour un déplacement vers la droite
 */
int move_lines (game *p, int direction){
    int line, 
		move = 0;

    for(line=0; line<(p->n); line++){
        move += move_line(p, line, direction);
    }
	
    return (move!=0);
}

/*!
 * Effectue les mouvements (vers le haut ou vers le bas) des cases d’une colonne.
 * Renvoie 1 si l’on a déplacé au moins une case, 0 sinon.
 *
 * \param p : pointeur sur un jeu
 * \param column : indice de colonne
 * \param direction : -1 pour déplacement vers la bas et 1 vers le haut
 */
int move_column (game *p, int column, int direction){
    int line, other_val, curr_val,
        size = (p->n-1),
        start = size*(direction==MOVE_RIGHT),
        limit = size*(direction==MOVE_LEFT),
        move = 0,
		pack = start;
    
	for(line=start; line!=(limit+direction); line+=direction){
		if(get_value(p, line, column)!=0){
			set_value(p, pack, column, get_value(p, line, column));
			if(pack!=line){
				set_value(p, line, column, 0);
				move++;
			}
			pack += direction;
        }
    } 

    for(line=start; line!=limit; line+=direction){
        other_val = get_value(p, line+direction, column);
        curr_val = get_value(p, line, column);

        if(other_val==curr_val && curr_val!=0){    
			move++;
    	    (p->free_slots)++;
            set_value(p, line, column, curr_val+other_val);

            for(pack=(line+direction); pack!=limit; pack+=direction){
                set_value(p, pack, column, get_value(p, pack+direction, column));
				set_value(p, pack+direction, column, 0);
            }
			set_value(p, pack, column, 0);
        }
    }
    return (move!=0);
}

/*!
 * Effectue les mouvements (vers le haut ou vers le bas) des cases de toutes les colonnes.
 * Renvoie 1 si l’on a déplacé au moins une case, 0 sinon.
 *
 * \param p : pointeur sur un jeu
 * \param direction : -1 pour déplacement vers la bas et 1 vers le haut
 */
int move_columns (game *p, int direction){
    int column, move = 0;

    for(column=0; column<(p->n); column++){
        move += move_column(p, column, direction);
    }
    return (move!=0);
}

/*!
 * Effectue le mouvement sur les lignes ou sur les colonnes suivant la valeur de direction.
 *
 * \param p : pointeur sur un jeu
 * \param direction : entier donnant la direction :
 * 0 : vers le bas
 * 1 : vers la droite
 * 2 : vers le haut
 * 3 : vers la gauche
 * Renvoie 1 si l’on a déplacé au moins une case, 0 sinon
 */
int move (game *p, int direction){
    
	switch(direction){
        case KEY_DOWN :
            return (move_columns(p, MOVE_DOWN));
            break;
        case KEY_RIGHT :
            return (move_lines(p, MOVE_RIGHT));
            break;
        case KEY_UP :
            return (move_columns(p, MOVE_UP));
            break;
        case KEY_LEFT :
            return (move_lines(p, MOVE_LEFT));
            break;
		default :
			return 0;
    }
}

/*!
 * Fonction permettant la saisie d’une direction ou de l’arrêt du jeu
 * (saisie répétée pour les autres touches)
 * Retourne :
 * -1 si l’utilisateur arrête le jeu
 * 0 si l’utilisateur souhaite déplacer vers le bas
 * 1 si l’utilisateur souhaite déplacer vers la droite
 * 2 si l’utilisateur souhaite déplacer vers le haut
 * 3 si l’utilisateur souhaite déplacer vers le gauche
 */
int input_read (){
	int arr_key;
    debutTerminalSansR();
    
	do{
        arr_key = (int)lectureFleche();
    }while(arr_key==NO_KEY);
	
    finTerminalSansR();                                             
    return arr_key;
}

/*!
 * Fonction permettant de jouer la partie en cours (on la suppose initialisée)
 * Retourne 1 si la partie est terminée (l’utilisateur a gagné ou perdu), et 0 sinon
 * (l’utilisateur a appuyé sur la touche Echap ou la touche s).
 */
int play (game *p){
	int direction;
	set_random_value(p);
    
	do{
	    show(p);
        direction = input_read();
		
		if(direction!=KEY_ESCAPE){
			//On ajoute une valeur aleatoire seulement s'il y a deplacement.
			if(move(p,direction)){
				set_random_value(p);
			}
		}

		if(game_over(p)){
			show(p);
		}                                
    }while(!game_over(p) && direction!=KEY_ESCAPE);

    return game_over(p);
}

int main(){
	game p;
	init_game(&p, 4, 2048);
	
	if(play(&p) && victory(&p)){
		printf("\nGagne!\n");
	}
	else if(defeat(&p)){
		printf("\nPerdu!\n");
	};
	printf("\nFin du jeu.\n");
	free_memory(&p);

    return 0;
}
