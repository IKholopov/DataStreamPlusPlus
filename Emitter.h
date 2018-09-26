#pragma once

#define interface struct

#include <assert.h>

template<typename DataT>
interface Emitter {
    virtual const DataT EmitNext() = 0;
    virtual bool Ended() = 0;
    virtual void Reset() = 0;

    virtual std::unique_ptr<Emitter> Copy() const = 0;
    virtual std::unique_ptr<Emitter> CopyState() const = 0;
};

template<typename DataT>
class EmitterGuard {
public:
    explicit EmitterGuard( Emitter<DataT>& emitter ): emitter(emitter) {}
    ~EmitterGuard() { emitter.Reset(); }

private:
    Emitter<DataT>& emitter;
};

template<typename DataT>
class EmitterIterator {
public:
    EmitterIterator(const Emitter<DataT>* original):
        emitter(std::move(original->Copy())),
        lastValue(nullptr)
    {
        updateLast();
    }

    EmitterIterator(const EmitterIterator& other):
        emitter(std::move(other.emitter->CopyState())),
        lastValue(other.lastValue == nullptr ? nullptr : new DataT(*other.lastValue.get()))
    {
    }

    const DataT& operator*() const {
        return *lastValue;
    }

    EmitterIterator& operator++() {
        updateLast();
        return* this;
    }

    EmitterIterator operator++(int) {
        EmitterIterator prev(*this);
        updateLast();
        return prev;
    }

    bool operator==(const EmitterIterator& other) const {
        return lastValue.get() == other.lastValue.get();
    }
    bool operator!=(const EmitterIterator& other) const {
        return lastValue.get() != other.lastValue.get();
    }

    EmitterIterator& operator=(const EmitterIterator& other) {
        if(this != &other) {
            emitter = std::move(other.emitter->CopyState());
            lastValue.reset(other.lastValue == nullptr ? nullptr : new DataT(*other.lastValue.get()));
        }
        return *this;
    }

private:
    std::unique_ptr< Emitter<DataT> > emitter;
    std::unique_ptr<const DataT> lastValue;

    void updateLast() { lastValue.reset(emitter->Ended() ? nullptr : new DataT(emitter->EmitNext()) ); }
};

template<typename DataT>
class EmptyEmitter : public Emitter<DataT>{
public:
    using iterator = EmitterIterator<DataT>;

    virtual const DataT EmitNext() { assert(false); }
    virtual bool Ended() { return true; }
    virtual void Reset() { assert(false); }

    virtual std::unique_ptr< Emitter<DataT> > Copy() const { return std::make_unique<EmptyEmitter<DataT>>(); }
    virtual std::unique_ptr< Emitter<DataT> > CopyState() const { return std::make_unique<EmptyEmitter<DataT>>(); }

    iterator begin() const { return iterator(this); }
    iterator end() const { return iterator(this); }
};
