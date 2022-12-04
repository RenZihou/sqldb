// record.h
// -*- encoding: utf-8 -*-
// @Author: RenZihou

#ifndef RECORD_H_
#define RECORD_H_


/**
 * @description a record in a table, it knows nothing about what the data really is
 */
class Record {
private:
    unsigned _page_id;
    unsigned _slot_id;
    char *_data;

public:
    Record();
    ~Record();
    Record(unsigned page_id, unsigned slot_id, char *data);
    unsigned getPageId() const;
    unsigned getSlotId() const;
    char *getData() const;
    void setPageId(unsigned page_id);
    void setSlotId(unsigned slot_id);
    void setData(char *data);
};


#endif  // RECORD_H_
