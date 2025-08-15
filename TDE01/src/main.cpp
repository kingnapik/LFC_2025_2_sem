#include <Arduino.h>

/*
L0 - todo 0 é seguido por 1
L1 - termina em 00
L2 - tem exatos 3 0
L3 - inicia em 1
L4 - não inicia em 1
*/

#define LED_L0 2
#define LED_L1 3
#define LED_L2 4
#define LED_L3 5
#define LED_L4 6

String input = "";
boolean complete = false;

void serialEvent(){
    while (Serial.available()){
        char c=(char)Serial.read();
        if (c=='\n'){ //esperando o enter
            complete=true;
        }else{
            input+=c;
        }
    }
}

bool eh1ou0(String s){
    for(int i=0;i<(int)s.length();i++){
        if (s[i] != '0' && s[i] != '1') return false;
    }
    return true;
}

bool verL0(String s){ //todo 0 é seguido de 1
    if (!eh1ou0(s)) return false;
    if (s.length()==0) return true; //se for vazia retorna verdaeiro
    for (int i=0;i<(int)s.length();i++){
        if(s[i]=='0'){
            bool tem1 = false;
            for(int j=i+1;j<(int)s.length();j++){
                if(s[j]=='1'){
                    tem1=true;
                    break;
                }else{
                    return false;
                }
            }
            if (tem1==false) return false;
        }
    }
    return true;
}

bool verL1(String s){ //termina em 00
    if (!eh1ou0(s)) return false;
    if (s.length()<2) return false;
    return (s[s.length()-2]=='0' && s[s.length()-1]=='0');
}

bool verL2(String s){ //3 0's
    if (!eh1ou0(s)) return false;
    int aux = 0;
    for (int i=0;i<(int)s.length();i++){
        if (s[i]=='0') aux++;
    }
    return(aux==3);
}

bool verL3(String s){ //começa em 1
    //if (s.length() == 0) return false; <-- redundante
    if (!eh1ou0(s)) return false;
    return(s[0]=='1');
}

bool verL4(String s){ // NAO começa em 1
    //if (s.length()==0) return true; <-- redundante
    if (!eh1ou0(s)) return false;
    return(s[0]!='1');
}

void verificaLingua(String s){
    Serial.println(s);
    digitalWrite(LED_L0, LOW);
    digitalWrite(LED_L1, LOW);
    digitalWrite(LED_L2, LOW);
    digitalWrite(LED_L3, LOW);
    digitalWrite(LED_L4, LOW);
    if (verL0(s)) digitalWrite(LED_L0, HIGH);
    if (verL1(s)) digitalWrite(LED_L1, HIGH);
    if (verL2(s)) digitalWrite(LED_L2, HIGH);
    if (verL3(s)) digitalWrite(LED_L3, HIGH);
    if (verL4(s)) digitalWrite(LED_L4, HIGH);
}

void setup(){
    Serial.begin(9600);
    pinMode(LED_L0, OUTPUT);
    pinMode(LED_L1, OUTPUT);
    pinMode(LED_L2, OUTPUT);
    pinMode(LED_L3, OUTPUT);
    pinMode(LED_L4, OUTPUT);
    Serial.println("String: ");
}

void loop(){
    if (complete){
        input.trim();
        verificaLingua(input);
        input="";
        complete=false;
    }
}