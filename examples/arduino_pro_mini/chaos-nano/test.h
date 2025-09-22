#ifndef _TEST_H_
#define _TEST_H_

class Test
{
  public:
    void setup(); 
    void on(bool b);
    void loop();
};

extern Test test;
#endif