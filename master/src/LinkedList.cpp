#include "Checkpoint.cpp"

class LinkedList{
    public:
        Checkpoint *head;
        int size;
        std::vector<Checkpoint> vectorForm;

        LinkedList(){}

        LinkedList(std::vector<Checkpoint> cpv){
            size = cpv.size();
            vectorForm = cpv;

            head = &cpv.at(0);
            Checkpoint *prev = nullptr;
            Checkpoint *current = head;

            for(int i = 0; i < cpv.size(); i++){
                if(i != cpv.size() - 1){
                    current->next = &cpv.at(i + 1);
                    current->prev = prev;
                    prev = current;
                }else{
                    current->prev = prev;
                }
            }
        }

        Checkpoint *iterate(int index){
            Checkpoint *current = head;
            for(int i = 0; i < index; i++){
                current = head->next;
            }
            return current;
        }

        void createLoop(Checkpoint *input, int index, int loopIndex){
            Checkpoint *prev = iterate(index);
            Checkpoint *next = iterate(index + 1);
            Checkpoint *extra = iterate(loopIndex);

            input->prev = prev;
            input->next = next;
            input->extra = extra;
            
            prev->next = input;
            next->prev = input;

            extra->extra = input;

            vectorForm.insert(vectorForm.begin() + index + 1, *input);
        }
};