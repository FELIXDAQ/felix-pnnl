#ifndef FLXEXCEPTION_H
#define FLXEXCEPTION_H

#include <stdexcept>
#include <sstream>
#include <string>
#include <iostream>
#include <sys/types.h>


//Macro
#define THROW_FLX_EXCEPTION(errorCode, message) throw FlxException(FlxException::errorCode, message);
       
class FlxException : public std::runtime_error 
{
public:  

  enum
  {
      NOTOPENED = 1,
      MAPERROR,
      UNMAPERROR,
      IOCTL,
      PARAM,
      I2C,
      SPI,
      GBT,
      REG_ACCESS,
      HW,
      NO_CODE
  };
  
  virtual const std::string getDescription() const;
  u_int getErrorId() const;
  virtual ~FlxException() throw () {} 

  FlxException(u_int errorId, std::string errorText);

protected:
   virtual std::string getErrorString(u_int errorId) const;

private:
  const u_int m_errorId;
  const std::string m_errorText;
  virtual std::ostream & print(std::ostream &stream) const;    
  std::ostream & printErrorMessage(std::ostream &stream) const;    
  std::ostream & printDescription(std::ostream &stream) const;    
};


inline std::ostream & FlxException::print(std::ostream &stream) const 
{
  return printErrorMessage(stream);  
}

#endif //FLXEXCEPTION_H
