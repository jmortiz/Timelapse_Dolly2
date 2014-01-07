#ifndef _DOLLY_STEPPER_
#define _DOLLY_STEPPER_

class DollyStepper{
  public:
    DollyStepper(unsigned char dir_pin, unsigned char step_pin);
    DollyStepper(unsigned char dir_pin, unsigned char step_pin, char enable_pin);
    void set_freq(unsigned int f);
    void set_steps_per_mm(float spmm);
    void set_direction(unsigned char d);
    unsigned int step(int nsteps); //sign of argument sets direction
    unsigned int move_mm(double mm_to_move);
    void move(char dir);
    void stop();
    unsigned int get_freq();
  private:
    unsigned char _step_pin;
    unsigned char _dir_pin;
    unsigned char _enable_pin;
    unsigned int _freq;
    double _steps_per_mm;
    char _step_direction;
};

#endif //_DOLLY_STEPPER_
