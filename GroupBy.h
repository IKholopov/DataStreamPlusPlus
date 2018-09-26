#pragma once

#include <memory>
#include <functional>
#include <unordered_set>

template<typename DataT, typename KeyT>
class GroupByEmitter : public Emitter<std::pair<KeyT, DataStream<DataT> > > {
public:
    GroupByEmitter(const Emitter<DataT>* emitter,
                   std::function<const KeyT( const DataT )> filter):
        emitter(std::move(emitter->Copy())),
        filter(filter),
        ended(false)
    {
    }

    GroupByEmitter(const GroupByEmitter& other):
        emitter(std::move(other.emitter->CopyState())),
        filter(other.filter),
        ended(other.ended)
    {
    }

    virtual const std::pair<KeyT, DataStream<DataT> > EmitNext() override {
        KeyT key = filter(emitter->EmitNext());
        for(;foundKeys.find(key) != foundKeys.end();
            key = filter(emitter->EmitNext()))
        {
        }
        foundKeys.insert({key});
        return {key, std::make_unique<const WhereEmitter<DataT>>(emitter.get(),
                     [f=filter, key](const DataT t){ return f(t) == key; }).get()};
    }

    virtual bool Ended() override {
        if(ended) {
            return true;
        }
        while(!emitter->Ended()) {
            if(foundKeys.find(filter(emitter->EmitNext())) == foundKeys.end()) {
                emitter->Reset();
                return false;
            }
        }
        emitter->Reset();
        ended = true;
        return true;
    }
    virtual void Reset() override {
        emitter->Reset();
        foundKeys.clear();
    }

    virtual std::unique_ptr<Emitter<std::pair<KeyT, DataStream<DataT> > >> Copy() const
        { return std::make_unique<GroupByEmitter>(emitter.get(), filter); }
    virtual std::unique_ptr<Emitter<std::pair<KeyT, DataStream<DataT> > >> CopyState() const
        { return std::make_unique<GroupByEmitter>(*this); }

private:
    std::unique_ptr< Emitter<DataT> > emitter;
    std::function<const KeyT( const DataT )> filter;
    std::unordered_set<KeyT > foundKeys;
    bool ended;
};
