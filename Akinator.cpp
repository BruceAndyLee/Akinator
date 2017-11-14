#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <cassert>
#include <cerrno>

using namespace std;

char lowerCase(char Chr)
{
    if (Chr >= 'A' && Chr <= 'Z')
		return Chr + ('a' - 'A'); 
	else
        return Chr;
}

char upperCase(char Chr)
{
    if (Chr >= 'a' && Chr <= 'z')
        return Chr - ('a' - 'A'); 
	else
        return Chr;
}

int mystrcmp(char* str1, char* str2) //strcasecmp
{
    for (int i = 0; str1[i] != '\0' && str2[i] != '\0'; i++)
        if (lowerCase(str1[i]) != lowerCase(str2[i]))
            return i + 1;
    return 0;
}

enum {MAX_NODE_STR_LEN = 128};

class Node
{
public:
    Node  ();
    Node  (char* data);
    Node  (Node* ancestor);
    Node  (char* data, Node* ancestor);
    Node  (Node* ancestor, Node* l_dec, Node* r_dec);
    Node  (char* data,     Node* l_dec, Node* r_dec);
    ~Node ();
    void  add_slashes();
    Node* ancestor_;
    Node* left_dec_;
    Node* right_dec_;
    char* data_;
};

Node::Node():
    data_      ((char*) calloc(MAX_NODE_STR_LEN, sizeof(char))),
    ancestor_  (NULL),
    left_dec_  (NULL),
    right_dec_ (NULL)
    {
		if (!data_)
		{
			printf ("Akinator: Node: error finding memory to keep data\n");
			exit(EXIT_FAILURE);
		}
    }

Node::Node(Node* ancestor):
    ancestor_   (ancestor),
    left_dec_   (NULL),
    right_dec_  (NULL)
    {
        data_ = (char*) calloc (MAX_NODE_STR_LEN, sizeof(char));
    }

Node::Node(char* data):
    ancestor_  (NULL),
    left_dec_  (NULL),
    right_dec_ (NULL),
    data_      (data)
    {
        //malloc + strncpy    
    }

Node::Node (char* data, Node* ancestor):
    ancestor_  (ancestor),
    left_dec_  (NULL),
    right_dec_ (NULL),
    data_      (data)
    {}

Node::Node  (Node* ancestor, Node* l_dec, Node* r_dec):
    ancestor_   (ancestor),
    left_dec_   (l_dec),
    right_dec_  (r_dec),
	data_		(NULL)
    {
        data_ = (char*) calloc (MAX_NODE_STR_LEN, sizeof(char));
        //what if null?
        left_dec_->ancestor_  = this;
        right_dec_->ancestor_ = this;
    }

Node::Node(char* data, Node* l_dec, Node* r_dec):
	ancestor_  (NULL),
    left_dec_  (l_dec),
    right_dec_ (r_dec), 
    data_      (data)
    {
        left_dec_ ->ancestor_ = this;
        right_dec_->ancestor_ = this;
    }

Node::~Node()
{
    if (data_)
        free(data_);//after every free assign with null

    if (left_dec_)
        delete left_dec_;

    if (right_dec_)
        delete right_dec_;
}

void Node::add_slashes()
{
    char* new_str = (char*) calloc (MAX_NODE_STR_LEN, sizeof(char));
    int k = 0;
    for (int i = 0; data_[i] != '\0'; i++)
    {
        if (data_[i] == ')' || data_[i] == '\\' || data_[i] == '(')
            new_str[k++] = '\\'; 
        new_str[k++] = data_[i];
    }
    free(data_);
    data_ = new_str;
}

enum TRAVERSE_MODE{
    PREF_ORDER,
    INF_ORDER,
    POSTF_ORDER,
};

class Akinator
{
public:
    Akinator(FILE* rw_file, Node* root);
    ~Akinator();
    void  visitor (Node* node_ptr, TRAVERSE_MODE mode, void (*act)(Node*));
    void  printTree             (Node* curNodePtr);
    void  accurateSymmetricPrint(Node* curNodePtr);
    void  inFilePrint_dot       ();
    void  printNodeInfo         (Node* curNodePtr);
    Node* buildTree             (Node* curNodePtr);
    int   searchNode            (char* wanted);
    void  interact              ();
    int   play                  (Node* curNodePtr);
    void  draw_menu             ();
    int   createNewNode         (Node* curNodePtr, char nodeId);
    Node* root_;
private:
    void  printNode             (Node* curNodePtr);
    void _inFilePrint           (Node* curNodePtr);
    void _inFilePrint_dot       (Node* curNodePtr, FILE* gv_f);
    int  _searchNode            (char* wanted, Node* curNodePtr);
    char* tree_str_;
    int   tree_offset_;
    FILE* rw_file_;
};

Akinator::Akinator(FILE* rw_file, Node* root):
    root_		(root),
	tree_str_	(NULL),
    tree_offset_(0),
    rw_file_	(rw_file)
    {
        if (!rw_file)
        {
            printf("\nAkinator: error: invalid file name\n");
            exit(EXIT_FAILURE);
        }
        size_t file_size = 0;
        fseek(rw_file_, 0, SEEK_END);
        file_size = (size_t) ftell(rw_file_);
        rewind(rw_file_);
        tree_str_ = (char*) calloc (file_size * 2, sizeof(char));
        if (!tree_str_)
        {
            printf("Akinator: error: cannot find memory to read file\n");
            exit(0);
        }
        if (fread(tree_str_, sizeof(char), file_size, rw_file_) < file_size)
			printf ("Akinator: building tree: Warning: read less characters than expected");

        rewind(rw_file_);
        printf("c_tor has done\n");
    }

Akinator::~Akinator()
{
    _inFilePrint(root_);
    tree_offset_   = 0;
    fclose(rw_file_);
    rw_file_ = NULL;
}

void Akinator::visitor(Node* node_ptr, TRAVERSE_MODE mode, void (*act)(Node*))
{
    switch(mode)
    {
        case PREF_ORDER:
            act(node_ptr);
            if (node_ptr->left_dec_)
                visitor(node_ptr->left_dec_,  mode, act);
            if (node_ptr->right_dec_)
                visitor(node_ptr->right_dec_, mode, act);
            break;
        case INF_ORDER:
            if (node_ptr->left_dec_)
                visitor(node_ptr->left_dec_,  mode, act);
            act(node_ptr);         
            if (node_ptr->right_dec_)
                visitor(node_ptr->right_dec_, mode, act);
            break;
        case POSTF_ORDER:
            if (node_ptr->left_dec_)
                visitor(node_ptr->left_dec_,  mode, act);
            if (node_ptr->right_dec_)
                visitor(node_ptr->right_dec_, mode, act);
            act(node_ptr);
            break;
		default:
			printf ("Visitor: error: unknown mode specifier %d\n", mode);
    }
}

void Akinator::printTree(Node* curNodePtr)
{
    printf("(");
    if(curNodePtr->data_)
       printf("%s", curNodePtr->data_);
    
    if(curNodePtr->left_dec_)
        printTree(curNodePtr->left_dec_);

    if(curNodePtr->right_dec_)
        printTree(curNodePtr->right_dec_);
    
    printf(")");
}

void Akinator::accurateSymmetricPrint(Node* curNodePtr)
{
    static int tree_level = 0;
    if(curNodePtr->left_dec_)
    {
        tree_level++;
        accurateSymmetricPrint(curNodePtr->left_dec_);
    }
    for(int i = 0; i < tree_level; i++)
        printf("\t");
    printf("'%s'\n", curNodePtr->data_);
    if(curNodePtr->right_dec_)
    {
        tree_level++;
        accurateSymmetricPrint(curNodePtr->right_dec_);
    }
    tree_level--;
}

void Akinator::_inFilePrint(Node* curNodePtr)
{
    curNodePtr->add_slashes();
    fprintf(rw_file_, "(%s", curNodePtr->data_);
    if(curNodePtr->left_dec_)
        _inFilePrint(curNodePtr->left_dec_);
    if(curNodePtr->right_dec_)
        _inFilePrint(curNodePtr->right_dec_);
    fprintf(rw_file_,")");
}

void Akinator::inFilePrint_dot()
{
    FILE* gv_f = fopen("Tree_file01.gv", "w");
    if (!gv_f)
    {
        printf("\nAkinator: error: cannot open .gv file\n");
        exit(1);
    };
    fprintf(gv_f, "digraph G{\n");
   _inFilePrint_dot(root_, gv_f);
    fprintf(gv_f, "\n}");
    fclose(gv_f);
    if (system("xdot Tree_file01.gv") < 0)
		printf ("Akinator: error evincing .gv file\n");
}

void Akinator::_inFilePrint_dot(Node* curNodePtr, FILE* gv_f)
{
    fprintf(gv_f, "_node_%p [label=\"%s\\l it's ptr %p\\l left_child_ptr %p\\l right_child_ptr %p\\l parent ptr %p\\l\"]\n", 
                                                           curNodePtr, curNodePtr->data_, 
                                                           curNodePtr, curNodePtr->left_dec_, 
                                                           curNodePtr->right_dec_,
                                                           curNodePtr->ancestor_);

    if (curNodePtr->left_dec_ && curNodePtr->right_dec_)
    {
        fprintf(gv_f, "_node_%p -> _node_%p\n", curNodePtr, curNodePtr->left_dec_);
        fprintf(gv_f, "_node_%p -> _node_%p\n\n", curNodePtr, curNodePtr->right_dec_);
        _inFilePrint_dot(curNodePtr->left_dec_,  gv_f);
        _inFilePrint_dot(curNodePtr->right_dec_, gv_f);
    }
}

Node* Akinator::buildTree(Node* curNodePtr)
{ 
    int stopCharPtr = 0;
    int data_offset = 0;
    tree_offset_++;    // move tree str to a char that's next to '('
    assert(MAX_NODE_STR_LEN == 128);
    while(sscanf(tree_str_ + tree_offset_, "%127[^\\()]%n", &curNodePtr->data_[data_offset], &stopCharPtr))
        switch (tree_str_[tree_offset_ + stopCharPtr])
        {
            case '\\':
                data_offset  += stopCharPtr;        //move data ptr at nearest empty cell
                tree_offset_ += (stopCharPtr + 1);  //move tree_str_ ptr at char that goes after '\'
                switch(*(tree_str_ + tree_offset_))
                {
#define ARGCASE(specialChar)                            \
case specialChar :                                      \
    *(curNodePtr->data_ + data_offset) = specialChar;   \
    tree_offset_++;                                     \
    data_offset++;                                      \
    break;
                    ARGCASE(')')
                    ARGCASE('(')
                    ARGCASE('\\')
#undef ARGCASE
                    default:
                        break;
                }
                break;

            case '(':
                tree_offset_ += stopCharPtr;

                printNode(curNodePtr);

                curNodePtr->left_dec_               = new Node(curNodePtr);
                curNodePtr->left_dec_               = buildTree(curNodePtr->left_dec_);
    
                curNodePtr->right_dec_              = new Node(curNodePtr);
                curNodePtr->right_dec_              = buildTree(curNodePtr->right_dec_);
                
                tree_offset_++; //skipping closing brace
                return  curNodePtr;
                break;
            case ')': 
                tree_offset_ += stopCharPtr + 1;
                return curNodePtr;
			default:
				printf ("Akinator: error parsing 'inline' tree: unknown ruling char '%c'\n", tree_str_[tree_offset_ + stopCharPtr]);
        }
}

int Akinator::createNewNode(Node* curNodePtr, char nodeId)
{
    switch(nodeId)
    {
        case 'L':
        {
            Node* new_left_dec     = new Node();
            Node* new_right_dec    = new Node(curNodePtr->left_dec_->data_);
            Node* new_node         = new Node(curNodePtr, new_left_dec, new_right_dec);
            curNodePtr->left_dec_  = new_node;
            
            printf("Tell me how is your character different from one I suggested?\n");
            while (scanf(" %[^\n]", new_node->data_) < 0)
				printf("Akinator: error interacting with user: %s\n", strerror(errno));

            strcat(new_node->data_, "?");
        
            printf("What answer was supposed to be given?\n");
			while (scanf(" %[^\n]", new_left_dec->data_) < 0)
				printf("akinator: error interacting with user: %s\n", strerror(errno));
    
            printf("Thank you for contribution!\n");
            printf("Would you like to go back to menu? (Y|N)\n");
            char answ;
           while ( scanf(" %c", &answ) < 0)
				printf("akinator: error interacting with user: %s\n", strerror(errno));
			
            if(upperCase(answ) == 'Y')
                    return 0;
            else
            {
                printf("Goodbye then.\n");
                inFilePrint_dot();
                _inFilePrint(root_);
                exit(EXIT_SUCCESS);
            }
        }
        case 'R':
        {
            Node* new_left_dec     = new Node();
            Node* new_right_dec    = new Node(curNodePtr->right_dec_->data_);
            Node* new_node         = new Node(curNodePtr, new_left_dec, new_right_dec);
            curNodePtr->right_dec_ = new_node;

            printf("Tell me how is your character different from one I suggested?\n");
            while (scanf(" %[^\n]", new_node->data_) < 0)
				printf("akinator: error interacting with user: %s\n", strerror(errno));

            strcat(new_node->data_, "?");
        
            printf("What answer was supposed to be given?\n");
            while (scanf(" %[^\n]", new_left_dec->data_) < 0)
				printf("akinator: error interacting with user: %s\n", strerror(errno));
    
            printf("Thank you for contribution!\n");
            printf("Would you like to go back to menu? (Y|N)\n");
            char answ;
            while (scanf(" %c", &answ) < 0)
				printf("akinator: error interacting with user: %s\n", strerror(errno));

            if(upperCase(answ) == 'Y')
                    return 0;
            else
            {
                printf("Goodbye then.\n");
                inFilePrint_dot();
                _inFilePrint(root_);
                exit(EXIT_SUCCESS);
            }
        }
		default:
			printf("Akinator: error creating new Node: unknown specifier %c; 'L' or 'R' expected\n", nodeId);
			exit(EXIT_FAILURE);
    }
}

int Akinator::play(Node* curNodePtr)
{
    printf("        %s (Y|N|D)\n", curNodePtr->data_);
    char answer = 0;
    while (scanf(" %c", &answer) < 0)
		printf("akinator: error interacting with user: %s\n", strerror(errno));

    answer = upperCase(answer);
    switch(answer)
    {
        case 'Y':
            if (!curNodePtr->left_dec_->left_dec_) //finds out if current node has only answer descentants
            {
                printf("Are you looking for '%s'? (Y|N|D)\n", curNodePtr->left_dec_->data_);
                while (scanf(" %c", &answer))
					printf("akinator: error interacting with user: %s\n", strerror(errno));
                switch(upperCase(answer))
                {
                    case 'Y':
                        printf("        Thank you for playing.\n");
                        return 0;
                
                    case 'M':
                        printf("    Game's been interrupted. Going back to menu.\n");
                        return 0;
                    case 'N':
                        return createNewNode(curNodePtr, 'L');                    
                    case 'D':
                        inFilePrint_dot();
                    default:
                        printf("    Unknown answer. You've just launched Earth extermination\n");
                        play(curNodePtr);
                }
            }
            else
                if(!play(curNodePtr->left_dec_))
                    return 0;

        case 'N':
            if (!curNodePtr->right_dec_->right_dec_){
                printf("    Are you looking for '%s'? (Y|N|D)\n", curNodePtr->right_dec_->data_);
                if (scanf(" %c", &answer) < 0)
				{
					printf("Akinator: error interacting with user: %s", strerror(errno));
				}
                switch(upperCase(answer))
                {
                    case 'Y':
                        printf("        Thank you for playing.\n");
                        return 0;
                
                    case 'M':
                        printf("    Game's been interrupted. Going back to menu.\n");
                        return 0;
                    case 'N':
                        return createNewNode(curNodePtr, 'R');                    
                    case 'D':
                        inFilePrint_dot();
                    default:
                        printf("    Unknown answer. You've just launched Earth extermination\n");
                        play(curNodePtr);
                }

            }
            else
                if(!play(curNodePtr->right_dec_))
                return 0;
        case 'M':
            printf("    Game's interrupted. Going back to menu");
            return 0;
        case 'D':
            inFilePrint_dot();
            play(curNodePtr);
        default:
            printf("    Unknown answer. You've just launched Earth extermination\n");
            play(curNodePtr);
    }
}

int Akinator::searchNode(char* wanted)
{
   return  _searchNode(wanted, root_);
}

int Akinator::_searchNode(char* wanted, Node* curNodePtr)
{
    if (!mystrcmp(curNodePtr->data_, wanted))
    {
        printNodeInfo(curNodePtr);
        return 1;
    }

    if (curNodePtr->left_dec_)
        if(int res = _searchNode(wanted, curNodePtr->left_dec_))
            return res;

    if (curNodePtr->right_dec_)
        if (int res = _searchNode(wanted, curNodePtr->right_dec_))
            return res;

    return 0;
}

void Akinator::draw_menu()
{
    printf("\n\n         __|===================================|__      \n");
    printf("        _|~~||           MENU:               ||~~|_     \n");
    printf("       _|~~~|| To play Akinator press    (1) ||~~~|_    \n");
    printf("      _|~~~~|| To find information press (2) ||~~~~|_   \n");
    printf("     _|~~~~~|| To quit this thing press  (3) ||~~~~~|_  \n");
    printf("    _|______===================================______|_ \n");
    printf("    ----------------To print Tree tad 'D'-------------- \n\n");
}

void Akinator::interact()
{
    printf("\n\n                  *Very polite greeting*\n\n");
    printf("&&&&I don't know coding very well so you'll have to press specified keys&&&&\n");
    while(1)
    {
        draw_menu();
        char specifier = 0;
        while (scanf(" %c", &specifier) < 0)
			printf("akinator: error interacting with user: %s\n", strerror(errno));

        switch(specifier)
        {
            case '1':
                play(root_);
                break;
            case '2':
            {
                char* wanted = (char*) calloc (MAX_NODE_STR_LEN, sizeof(char));
                printf("    Who would you like to find\n");
                while (scanf(" %[^\n]", wanted) < 0)
					printf("akinator: error interacting with user: %s\n", strerror(errno));

                if (!searchNode(wanted))
                    printf("    Didn't find anything.\n");                 
                printf("    Would you like to go back to menu?\n");
                char answ;
                while (scanf(" %c", &answ) < 0)
					printf("akinator: error interacting with user: %s\n", strerror(errno));
					
                if (upperCase(answ) == 'Y')
                    break;
                else
                {
                    printf("Goodbye then.\n");
                    inFilePrint_dot();
                    exit(0);
                }
            }
            case '3':
                printf("Goodbye then.\n");
                _inFilePrint(root_); //before quiting writes in file
                inFilePrint_dot();  //           and in .gv file
                exit(0);
            case 'D':
                printf("GOT D GONNA SHOW TREE\n");
                inFilePrint_dot();
                break;
            default:
                printf("Unknown key\n");
        }
    }
}

void Akinator::printNodeInfo(Node* node)
{
    static int print_space = 0;
    if(!print_space)
    {
        printf("node info: %s\n", node->data_);
        print_space = 1;
    }
    else
    {
        char* infoStr = strdup(node->data_);
        infoStr[strlen(infoStr) - 1] = '\0';
        printf("    %s\n", infoStr);
    }
    print_space = 1;
    if (node->ancestor_)
        printNodeInfo(node->ancestor_);
}

void Akinator::printNode(Node* curNodePtr)
{
    printf("\n");
    printf("Node pointer is %p\n",      curNodePtr);
    if (curNodePtr->data_)
        printf("Node data      '%s'\n",     curNodePtr->data_);

    printf("Left descendant  ptr %p\n", curNodePtr->left_dec_);
    printf("Right descendant ptr %p\n", curNodePtr->right_dec_);
    printf("Ancestor ptr         %p\n", curNodePtr->ancestor_);
    printf("\n");
}

int main (int argc, char* argv[])
{
	if (argc < 2)
	{
        printf("Usage: %s [tree file]\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    FILE* frw = NULL;
    if (!(frw = fopen(argv[1], "r+")))
    {
        printf("\nError: cannot open tree file\n");
        exit(EXIT_FAILURE);
    };
    Node root1;
    Akinator rakinator(frw, &root1);
    rakinator.root_ = rakinator.buildTree(rakinator.root_);
    printf("BUILDED\n");
    rakinator.accurateSymmetricPrint(&root1);

    rakinator.interact();
    return 0;
}
