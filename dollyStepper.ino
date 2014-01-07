DollyStepper::DollyStepper(unsigned char dir_pin, unsigned char step_pin, char enable_pin)
{
  _step_pin = step_pin;
  _dir_pin = dir_pin;
  _enable_pin = enable_pin;
  _freq = 200;//163;
  _steps_per_mm = 161.3;
  _step_direction = 1;
  
  pinMode(dir_pin, OUTPUT);
  pinMode(step_pin, OUTPUT);
  pinMode(enable_pin, OUTPUT);
  
  if(enable_pin > 0)
    digitalWrite(enable_pin,HIGH); //disable stepper
}


DollyStepper::DollyStepper(unsigned char dir_pin, unsigned char step_pin)
{
  DollyStepper(dir_pin, step_pin, -1);
}

void DollyStepper::set_freq(unsigned int f)
{
  _freq = f;
}

unsigned int DollyStepper::get_freq()
{
  return _freq;
}

void DollyStepper::set_direction(unsigned char d)
{
  if(d > 0)
    _step_direction = 1;
  else if (d < 0)
    _step_direction = -1;
}

void DollyStepper::set_steps_per_mm(float spmm)
{
  _steps_per_mm = spmm;
}

unsigned int DollyStepper::step(int nsteps)
{
  if(_enable_pin > 0)
    digitalWrite(_enable_pin,LOW); //enable
  
  if (nsteps*_step_direction > 0)
    digitalWrite(_dir_pin, HIGH);
  else if (nsteps*_step_direction < 0)
    digitalWrite(_dir_pin, LOW);
  float duration = 1000*fabs(nsteps)/(float)_freq;
  //long int duration = (1000*abs(nsteps))/_freq;
  
  Serial.println(String(nsteps)+" steps, duration: "+String((int)duration)+"ms");
  
  tone(_step_pin, _freq, duration);
  
  return duration;
}

void DollyStepper::move(char dir)
{
  if(_enable_pin > 0)
    digitalWrite(_enable_pin,LOW); //disable stepper
  if (dir*_step_direction > 0)
    digitalWrite(_dir_pin, HIGH);
  else if (dir*_step_direction < 0)
    digitalWrite(_dir_pin, LOW);
  
  tone(_step_pin, _freq);
}
void DollyStepper::stop()
{
  noTone(_step_pin);
  if(_enable_pin > 0)
    digitalWrite(_enable_pin,HIGH); //disable stepper
}

unsigned int DollyStepper::move_mm(double mm_to_move)
{
  return step(_steps_per_mm*mm_to_move);
}
