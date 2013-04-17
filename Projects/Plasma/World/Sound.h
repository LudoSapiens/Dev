/*=============================================================================
   Copyright (c) 2012, Ludo Sapiens Inc. and contributors.
   See accompanying file LICENSE.txt for details.
=============================================================================*/
#ifndef PLASMA_SOUND_H
#define PLASMA_SOUND_H

#include <Plasma/StdDefs.h>

#include <Fusion/VM/VMObject.h>

#include <Snd/Listener.h>
#include <Snd/Sound.h>
#include <Snd/Source.h>

#include <Base/Msg/Observer.h>

NAMESPACE_BEGIN

class Entity;

/*==============================================================================
  CLASS Sound
==============================================================================*/
class Sound:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API Sound();
   PLASMA_DLL_API virtual ~Sound();

   // VM
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API virtual bool performGet( VMState* );
   PLASMA_DLL_API virtual bool performSet( VMState* );

   // Snd::Buffer interface.
   inline       Snd::Freq  freq()        const { return _sound->freq();        }
   inline Snd::SampleType  sampleType()  const { return _sound->sampleType();  }
   inline            uint  numChannels() const { return _sound->numChannels(); }
   inline          size_t  sizeInBytes() const { return _sound->sizeInBytes(); }
   inline          double  getDuration() const { return _sound->getDuration(); }

   inline const RCP<Snd::Sound>&  sound() const { return _sound; }

protected:
   RCP<Snd::Sound>  _sound;

private:
}; //class Sound


/*==============================================================================
  CLASS SoundSource
==============================================================================*/
class SoundSource:
   public RCObject,
   public Observer
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SoundSource();
   PLASMA_DLL_API virtual ~SoundSource();

   // VM
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API virtual bool performGet( VMState* );
   PLASMA_DLL_API virtual bool performSet( VMState* );

   // Observer interface.
   PLASMA_DLL_API virtual void  update();

   PLASMA_DLL_API void  observe( Entity* s );

   // Source interface.
   inline        float  pitch()                               const { return _source->pitch();      }
   inline         void  pitch( const float pitch )                  { _source->pitch(pitch);        }
   inline        float  gain()                                const { return _source->gain();       }
   inline         void  gain( const float gain )                    { _source->gain(gain);          }
   inline       Vec3f&  position()                                  { return _source->position();   }
   inline const Vec3f&  position()                            const { return _source->position();   }
   inline         void  position( const Vec3f& p )                  { _source->position(p);         }
   inline       Vec3f&  velocity()                                  { return _source->velocity();   }
   inline const Vec3f&  velocity()                            const { return _source->velocity();   }
   inline         void  velocity( const Vec3f& v )                  { _source->velocity(v);         }
   inline         bool  looping()                             const { return _source->looping();    }
   inline         void  looping( bool v )                           { _source->looping(v);          }
   inline         bool  relative()                            const { return _source->relative();   }
   inline         void  relative( bool v )                          { _source->relative(v);         }
   inline         bool  autoDelete()                          const { return _source->autoDelete(); }
   inline         void  autoDelete( bool v )                        { _source->autoDelete(v);       }
   inline         void  bind( const Snd::Sound* sound )             { _source->bind(sound);         }
   inline         void  setDefaults()                               { _source->setDefaults();       }
   inline         void  setBGM()                                    { _source->setBGM();            }
   //inline       bool  update()                                    { return _source->update();     }
   inline         bool  play()                                      { return _source->play();       }
   inline         bool  pause()                                     { return _source->pause();      }
   inline         bool  stop()                                      { return _source->stop();       }
   inline         bool  rewind()                                    { return _source->rewind();     }
   inline         bool  playing()                                   { return _source->playing();    }
   inline         bool  stopped()                                   { return _source->stopped();    }

   // Additional convenience routines.
   inline         void  setSound( const Sound* s ) { _source->bind(s->sound().ptr()); }

   inline const RCP<Snd::Source>&  source() const { return _source; }

protected:

   /*----- data members -----*/

   RCP<Snd::Source>  _source;
   Entity*           _entity;

   /*----- methods -----*/

   virtual void  destroy();

private:
}; //class SoundSource


/*==============================================================================
  CLASS SoundListener
==============================================================================*/
class SoundListener:
   public RCObject,
   public Observer

{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SoundListener();
   PLASMA_DLL_API virtual ~SoundListener();

   // VM
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API virtual bool performGet( VMState* );
   PLASMA_DLL_API virtual bool performSet( VMState* );

   // Observer interface.
   PLASMA_DLL_API virtual void  update();

   PLASMA_DLL_API void  observe( Entity* e );

   // Listener interface.
   inline        float  gain()                        const { return _listener->gain();            }
   inline        void   gain( const float g )               { _listener->gain(g);                  }
   inline       Vec3f&  position()                          { return _listener->position();        }
   inline const Vec3f&  position()                    const { return _listener->position();        }
   inline         void  position( const Vec3f& p )          { _listener->position(p);              }
   inline       Vec3f&  velocity()                          { return _listener->velocity();        }
   inline const Vec3f&  velocity()                    const { return _listener->velocity();        }
   inline         void  velocity( const Vec3f& v )          { _listener->velocity(v);              }
   inline       Vec3f&  pointOfInterest()                   { return _listener->pointOfInterest(); }
   inline const Vec3f&  pointOfInterest()             const { return _listener->pointOfInterest(); }
   inline         void  pointOfInterest( const Vec3f& poi ) { _listener->pointOfInterest(poi);     }
   inline       Vec3f&  upVector()                          { return _listener->upVector();        }
   inline const Vec3f&  upVector()                    const { return _listener->upVector();        }
   inline         void  upVector( const Vec3f& up )         { _listener->upVector(up);             }
   //inline       bool  update()                            { return _listener->update();          }
   inline         void  setDefaults()                       { _listener->setDefaults();            }

   inline const RCP<Snd::Listener>&  listener() const { return _listener; }

protected:

   /*----- data members -----*/

   RCP<Snd::Listener>  _listener;
   Entity*             _entity;

   /*----- methods -----*/

   virtual void  destroy();

private:
}; //class SoundListener


/*==============================================================================
  CLASS SoundManager
==============================================================================*/
class SoundManager:
   public RCObject
{
public:

   /*----- methods -----*/

   PLASMA_DLL_API SoundManager();
   PLASMA_DLL_API virtual ~SoundManager();

   // Sound generation routines.
   PLASMA_DLL_API void genCollision( const Vec3f& position, const float force /* matA, matB*/ );

   // VM
   PLASMA_DLL_API void init( VMState* );
   PLASMA_DLL_API virtual bool performGet( VMState* );
   PLASMA_DLL_API virtual bool performSet( VMState* );

   const RCP<Snd::Manager>&  sndMgr() { return _sndMgr; }

protected:

   /*----- data members -----*/

   RCP<Snd::Manager>  _sndMgr;

}; //class SoundManager


/*==============================================================================
  VM Section
  ==============================================================================*/

VMOBJECT_TRAITS( Sound, sound )
typedef VMObject< Sound > SoundVM;

VMOBJECT_TRAITS( SoundSource, source )
typedef VMObject< SoundSource > SoundSourceVM;

VMOBJECT_TRAITS( SoundListener, listener )
typedef VMObject< SoundListener > SoundListenerVM;

VMOBJECT_TRAITS( SoundManager, manager )
typedef VMObject< SoundManager > SoundManagerVM;


NAMESPACE_END

#endif //PLASMA_SOUND_H
