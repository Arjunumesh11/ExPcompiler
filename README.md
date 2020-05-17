## INSTALLATION
____

### Install LEX

For Ubuntu Users :
```bash
sudo apt-get update
sudo apt-get install flex
```
For Fedora Users :
```bash
yum install flex
```
### Install YACC

For Ubuntu Users :
```bash
sudo apt-get update
sudo apt-get install bison
```

For Fedora Users :
```bash
yum install bison
```
### Install XSM Machine Simulator

The compiler's target code needs to be run on a simulator and the installation steps are given below.

Let us install XSM
Step 1 :

Download XSM Simulator : [Version 1](https://silcnitc.github.io/files/xsm_expl.tar.gz)

Step 2 :

Extract this file and navigate into the folder xsm_expl through the terminal.

Do the following steps

    1. Type "make".

    You may get some warnings and they can be ignored. If you get any fatal error, then install the following dependencies and try running "make" again :
    sudo apt-get install libreadline-dev
    sudo apt-get install libc6-dev
    If any other dependencies are missing (this depends on your system configuration), you have to install the missing dependencies and run "make" again. (Optional) While running "./xsm" after Step 2 if you get this error:
    /usr/bin/ld: cannot find -ll collect2: error: ld returned 1 exit status Then you need to install:
    sudo apt-get install libfl-dev
    and edit the Makefile of xsm_dev folder, to proceed find the line where "-ll" is used as option and update it to "-lfl" to use the "flex" library we installed above. Now you can run "make" again after navigating into the folder xsm_expl through the terminal.
    2. Type "cd ../xfs-interface/" and type "./init".
    3.(Optional) Add this line #!/usr/bin/env bash as first line to the xsm file in xsm_expl folder.
    This step is for those who don't have bash or sh as their "default shell" and therefore may be using other customizable shells like zsh, etc as their default shell. You can check your default shell by using echo $SHELL in terminal. 
### 
The usage instructions for the XSM simulator can be found [here](https://silcnitc.github.io/xsmusagespec.html).

## Generating Binary
___
```bash
yacc grammer.y -d
lex parser.l
gcc y.tab.c lex.yy.c codegenerator.c -o expl
lex label.l
gcc yy.lex.c -o linker
```
## Usage
```bash
./expl <input_file.txt>
./linker output.txt
./xsm_expl/xsm -e ../output.xsm
```
## Test program
```c
type
	bst{
           int a;
           bst left;
           bst right;
	    }
endtype
	
class
	bstclass{
        decl
           bst root;
           int init();
           bst getroot();
           int setroot(bst n1);
           bst getnode(int key);
           bst insert(bst h, int key);
           int inOrder_fun(bst h);
           int preOrder_fun(bst h);
           int postOrder_fun(bst h);
        enddecl
        int init(){
           begin
              self.root=null;
              return 1;
           end
        }
        bst getroot(){
           begin
              return self.root;
              end
        }
        int setroot(bst n1){
           begin
              self.root=n1;
              return 1;
           end
        }
        bst getnode(int key){
           decl
              bst temp;
              enddecl
           begin
              temp=alloc();
              temp.a=key;
              temp.left=null;
              temp.right=null;
              return temp;
           end
        }
        
        bst insert(bst h, int key){
           begin
              if (h == null) then
                 h = self.getnode(key);
              else
                 if (key < h.a) then
                    h.left = self.insert(h.left, key);
                else
                    if (key > h.a) then
                    h.right = self.insert(h.right, key);
                    endif;
                endif;
              endif;
              return h;
           end
        }
        int inOrder_fun(bst h){
           decl
              int in;
              enddecl
           begin
              if(h!= null) then
              in=self.inOrder_fun(h.left);
              write(h.a);
              in=self.inOrder_fun(h.right);
              endif;
              return 1;
           end
        }
        
        int preOrder_fun(bst h){
           decl
              int in;
           enddecl
           begin
              if(h!= null) then
              write(h.a);
              in=self.preOrder_fun(h.left);
              in=self.preOrder_fun(h.right);
              endif;
              return 1;
           end
           }
           
        int postOrder_fun(bst h){
           decl
              int in;
           enddecl
           begin
           
           if(h!= null) then
           in=self.postOrder_fun(h.left);
           in=self.postOrder_fun(h.right);
           write(h.a);
           endif;
           return 1;
           end
        
        }
        
	}
	endclass
	
    decl
        bstclass obj;
	enddecl
	int main(){
        decl
           bst Root;
           int x,in,val;
        enddecl
        begin
           x=initialize();
           obj = new(bstclass);
           x=obj.init();
           read(val);
           Root = obj.getroot();
           while(val!=0) do
              Root = obj.insert(Root,val);
              read(val);
           endwhile;
           x = obj.setroot(Root);
           in = obj.inOrder_fun(obj.getroot());
           in = obj.preOrder_fun(obj.getroot());
           in = obj.postOrder_fun(obj.getroot());
           return 0;
        end
	}

