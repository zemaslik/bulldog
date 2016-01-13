package io.silverspoon.bulldog.core.gpio.base;

import io.silverspoon.bulldog.core.Edge;
import io.silverspoon.bulldog.core.Signal;
import io.silverspoon.bulldog.core.event.InterruptEventArgs;
import io.silverspoon.bulldog.core.event.InterruptListener;
import io.silverspoon.bulldog.core.gpio.DigitalInput;
import io.silverspoon.bulldog.core.pin.AbstractPinFeature;
import io.silverspoon.bulldog.core.pin.Pin;

import java.util.ArrayList;
import java.util.Collections;
import java.util.List;

public abstract class AbstractDigitalInput extends AbstractPinFeature implements DigitalInput {

   private static final int MAX_DEBOUNCE_COUNT = 10;

   private static final String NAME_FORMAT = "Digital Input on Pin %s";
   private Edge trigger = Edge.Both;
   private List<InterruptListener> interruptListeners = Collections.synchronizedList(new ArrayList<InterruptListener>());
   private int debounceMs = 0;
   private boolean areInterruptsEnabled = true;

   public AbstractDigitalInput(Pin pin) {
      super(pin);
   }

   public String getName() {
      return String.format(NAME_FORMAT, getPin().getName());
   }

   public void setInterruptTrigger(Edge edge) {
      this.trigger = edge;
   }

   public void setInterruptDebounceMs(int milliSeconds) {
      this.debounceMs = milliSeconds;
   }

   public int getInterruptDebounceMs() {
      return debounceMs;
   }

   public Edge getInterruptTrigger() {
      return this.trigger;
   }

   public void addInterruptListener(InterruptListener listener) {
      interruptListeners.add(listener);
   }

   public void removeInterruptListener(InterruptListener listener) {
      interruptListeners.remove(listener);
   }

   public List<InterruptListener> getInterruptListeners() {
      return interruptListeners;
   }

   public void clearInterruptListeners() {
      interruptListeners.clear();
   }

   public void fireInterruptEvent(InterruptEventArgs args) {
      synchronized (interruptListeners) {
         if (areInterruptsEnabled() == false) {
            return;
         }
         for (InterruptListener listener : interruptListeners) {
            listener.interruptRequest(args);
         }
      }
   }

   public Signal readDebounced(int debounceTime) {
      long startTime = System.currentTimeMillis();
      long delta = 0;
      Signal currentState = read();
      int counter = 0;
      while (delta < debounceTime) {
         Signal reading = read();

         if (reading == currentState && counter > 0) {
            counter--;
         }

         if (reading != currentState) {
            counter++;
         }

         if (counter >= MAX_DEBOUNCE_COUNT) {
            counter = 0;
            currentState = reading;
            return currentState;
         }

         delta = System.currentTimeMillis() - startTime;
      }

      return currentState;
   }

   public boolean areInterruptsEnabled() {
      return areInterruptsEnabled;
   }

   public void enableInterrupts() {
      enableInterruptsImpl();
      areInterruptsEnabled = true;
   }

   public void disableInterrupts() {
      disableInterruptsImpl();
      areInterruptsEnabled = false;
   }

   protected abstract void enableInterruptsImpl();

   protected abstract void disableInterruptsImpl();
}
