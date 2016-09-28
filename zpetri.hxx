/* ------------------------------------------------------------------------- */
/*  ������ ���� �������� ������ ��������� �������� �������,                  */
/*  �������������� �������� ���������� �������:                              */
/*  ������� �.�.                                                             */
/*  ������ ������������� ����������������. - �.: �����-�����, 2012. - 384 �. */
/*  ISBN 978-5-91359-102-9                                                   */
/*                                                                           */
/*  ��� � ���� �������, ����������� � ���� �������� ������ �������������     */
/*  ���� ��� ������������ � ���������� ���������������� ����������           */
/*  ������������ ��������, � ����� ��� ���������� ���������� �������������.  */
/*  ������������� ����� ���� � �������� ������ ��� �������� ��������         */
/*  ���������, ������ ������� ��������� � ����� �������������� ����          */
/*  �� ������ ����� � ���� ������������.                                     */
/*  �������� ������ ��������������� "��� ����", ��� ����� �� �� �� ����      */
/*  ����� ��� ������� �������� ����������� � ������������� ����������.       */
/*                                                                           */
/*  Copyright � 2008-2011 ������� �.�.                                       */
/* ------------------------------------------------------------------------- */


#ifndef _ZPETRI_HXX_
#define _ZPETRI_HXX_

#include <vector>
#include <map>
#include <iterator>
#include <cassert>

namespace z {
namespace petri {

// ������ ��� ��� ����������� �������
class place_type {};

// ����������� ��� ��������
class transition_abstract_type
{
public:
    // ������ ����������� ���������
    typedef std::vector<transition_abstract_type *> enabledlist_type;
    // ������ ���������� �������
    typedef std::map<place_type *, int> markedlist_type;

    // ��������� ��������
    virtual
    void activate(void) = 0;
    // ��������� ����������, ������� �� �������
    virtual
    bool is_active(void) const = 0;
    // ��������� ������ ���������� ����������� ���������
    // (������ ���� ������� ������� �������)
    virtual
    enabledlist_type get_enabled(void) const = 0;
    // ��������� ������ ���������� ���������� �������
    // (������ ���� ������� ������� �������)
    virtual
    markedlist_type get_marked(void) const = 0;
    // ������������ ������ �� ����������� ���������� ���������
    // (������ ���� ������� ������� �������)
    virtual
    void fire(int number) = 0;

    // ����������� �������
    virtual
    void on_activate(void) {}
    virtual
    void on_passivate(void) {}
};

// ������� (���������) �������
class transition_simple_type: public transition_abstract_type
{
public:
    void activate(void)
    {}
    bool is_active(void) const
    { return false; }
    enabledlist_type get_enabled(void) const
    { return enabledlist_type(); }
    markedlist_type get_marked(void) const
    { return markedlist_type(); }
    void fire(int)
    { assert(false); }
};

// ��������� ������� (��������� ���� �����)
class transition_compound_type: public transition_abstract_type
{
private:
    typedef std::vector<place_type *> placelist_type;
    typedef std::vector<transition_abstract_type *> transitionlist_type;
    typedef std::vector<int> marking_type;
    typedef std::vector<marking_type> arcmatrix_type;

public:
    // ���������� ���� �����
    class content_type
    {
    private:
        typedef std::map<place_type *, int> plmap_type;
        typedef std::map<transition_abstract_type *, int> trmap_type;
        typedef std::map<std::pair<int, int>, int> arcmap_type;
        typedef std::map<int, int> tokmap_type;

        // ����������� ������� �� �� ������
        plmap_type m_plmap;
        // ����������� ��������� �� �� ������
        trmap_type m_trmap;
        // ����������� ������� � �������� ��� �� �� ����
        arcmap_type m_inmap, m_outmap;
        // ����������� ������� ������� �� ���������� �����
        tokmap_type m_tokmap;

    private:
        // ���������� ���� ��������� ���������
        void add_arc(
            place_type &pl,
            transition_abstract_type &tr,
            int weight,
            arcmap_type &arcmap)
        {
            assert(m_plmap.find(&pl) != m_plmap.end());
            assert(m_trmap.find(&tr) != m_trmap.end());
            assert(weight > 0);
            arcmap_type::key_type arc(m_trmap[&tr], m_plmap[&pl]);
            arcmap[arc] += weight;
        }
        // ���������� ������� ��������� ���
        arcmatrix_type build_matrix(const arcmap_type &arcmap) const
        {
            arcmatrix_type mtx(m_trmap.size());
            arcmatrix_type::iterator it;
            for (it = mtx.begin(); it != mtx.end(); ++it)
            {
                arcmatrix_type::value_type vec(m_plmap.size());
                arcmatrix_type::value_type::iterator jt;
                for (jt = vec.begin(); jt != vec.end(); ++jt)
                {
                    arcmap_type::const_iterator ft;
                    ft = arcmap.find(arcmap_type::key_type(
                        std::distance(mtx.begin(), it),
                        std::distance(vec.begin(), jt)));
                    *jt = (ft != arcmap.end()) ? ft->second : 0;
                };
                *it = vec;
            };
            return mtx;
        }

    public:

        // ------- �������, ������������ ��� ���������� ���� -------

        // ���������� �������
        void add_place(place_type &pl)
        {
            assert(m_plmap.find(&pl) == m_plmap.end());
            m_plmap.insert(plmap_type::value_type(&pl, m_plmap.size()));
        }

        // ���������� ��������
        void add_transition(transition_abstract_type &tr)
        {
            assert(m_trmap.find(&tr) == m_trmap.end());
            m_trmap.insert(trmap_type::value_type(&tr, m_trmap.size()));
        }

        // ���������� ������� ����
        void add_arc(
            place_type &in,
            transition_abstract_type &tr,
            int weight = 1)
        {
            add_arc(in, tr, weight, m_inmap);
        }
        // ���������� �������� ����
        void add_arc(
            transition_abstract_type &tr,
            place_type &out,
            int weight = 1)
        {
            add_arc(out, tr, weight, m_outmap);
        }

        // ���������� ���������� ���������� ����� � �������
        void add_token(place_type &pl, int tokens = 1)
        {
            assert(tokens > 0);
            plmap_type::iterator it = m_plmap.find(&pl);
            assert(it != m_plmap.end());
            m_tokmap[it->second] += tokens;
        }

        // ------- �������, ������������ ����� -------

        // ��������� ������ �������
        placelist_type get_pllist(void) const
        {
            placelist_type pllist(m_plmap.size());
            plmap_type::const_iterator it;
            for (it = m_plmap.begin(); it != m_plmap.end(); ++it)
                pllist[it->second] = it->first;
            return pllist;
        }

        // ��������� ������ ���������
        transitionlist_type get_trlist(void) const
        {
            transitionlist_type trlist(m_trmap.size());
            trmap_type::const_iterator it;
            for (it = m_trmap.begin(); it != m_trmap.end(); ++it)
                trlist[it->second] = it->first;
            return trlist;
        }

        // ��������� ������ ������� � �������� ���
        arcmatrix_type get_inmatrix(void) const
        {
            return build_matrix(m_inmap);
        }
        arcmatrix_type get_outmatrix(void) const
        {
            return build_matrix(m_outmap);
        }

        // ��������� ��������� ��������
        marking_type get_marking(void) const
        {
            marking_type marking(m_plmap.size(), 0);
            tokmap_type::const_iterator it;
            for (it = m_tokmap.begin(); it != m_tokmap.end(); ++it)
                marking[it->first] = it->second;
            return marking;
        }
    };

private:
    // ������ �������
    placelist_type m_pllist;
    // ������ ���������
    transitionlist_type m_trlist;
    // ������� ������� � �������� ���
    arcmatrix_type m_mtxin;
    arcmatrix_type m_mtxout;
    // ��������� ��������
    marking_type m_marking_init;

    // ������� ��������
    marking_type m_marking;
    // ������ ������ ���������� �������, ������� ����������
    markedlist_type m_marked;
    // ������ ������ ����������� ���������, ������� ����������
    enabledlist_type m_enabled;
    // ����������� ��������� m_enabled �� ��������� ��������
    std::vector<int> m_location;
    // �������� � m_enabled ������� ������� ���������� ��������
    std::vector<int> m_offset;

    // ���������� ��������� �������
    int pl_num(void) const
    {
        return m_pllist.size();
    }
    // ���������� ��������� ���������
    int tr_num(void) const
    {
        return m_trlist.size();
    }

    // ���������� ������� ����������� ��������� � ���������� �������
    void refresh(void)
    {
        // ������� ��� ����������� �������� � �� ����������
        m_enabled.clear();
        m_location.clear();
        m_offset.resize(tr_num());
        for (int i = 0; i < tr_num(); ++i)
        {
            // ������������ � m_enabled ������� i-�� ���������� ��������
            m_offset[i] = m_enabled.size();

            // ���� ������� �������
            if (m_trlist[i]->is_active())
            {
                // ������� ���������� ����������� ��������
                enabledlist_type inner = m_trlist[i]->get_enabled();
                // ������� �� � ����� �������� ������
                m_enabled.insert(
                    m_enabled.end(), inner.begin(), inner.end());
                // ��� ��� ����������� � i-� ��������� ��������
                m_location.insert(m_location.end(), inner.size(), i);
            }
            else
            {
                // ���� �� �������, �������, �������� ��
                bool isenabled = true;
                for (int j = 0; isenabled && j < pl_num(); ++j)
                    isenabled = (m_marking[j] >= m_mtxin[i][j]);
                // � ������� ���, ���� ��������
                if (isenabled)
                {
                    m_enabled.push_back(m_trlist[i]);
                    m_location.push_back(i);
                };
            };
        };

        // ������� ��� ���������� �������
        m_marked.clear();
        // ��������� �������
        for (int j = 0; j < pl_num(); ++j)
            if (m_marking[j] > 0)
                m_marked[m_pllist[j]] = m_marking[j];
        // ���������� ������� ��������� �������� ���������
        for (int i = 0; i < tr_num(); ++i)
        {
            if (m_trlist[i]->is_active())
            {
                markedlist_type inner = m_trlist[i]->get_marked();
                m_marked.insert(inner.begin(), inner.end());
            };
        };
    }

public:
    explicit
    transition_compound_type(const content_type &content):
        m_pllist(content.get_pllist()),
        m_trlist(content.get_trlist()),
        m_mtxin(content.get_inmatrix()),
        m_mtxout(content.get_outmatrix()),
        m_marking_init(content.get_marking())
    {}

    void activate(void)
    {
        m_marking = m_marking_init;
        refresh();
    }
    bool is_active(void) const
    {
        return !m_enabled.empty();
    }
    enabledlist_type get_enabled(void) const
    {
        return m_enabled;
    }
    markedlist_type get_marked(void) const
    {
        return m_marked;
    }
    void fire(int number)
    {
        assert(number >= 0 && size_t(number) < m_enabled.size());

        // ������ ���������� �������������� �������� � ���������
        int local = m_location[number];
        // � ���������� ����� (���� ��������� ��������� �������)
        int lower = number - m_offset[local];

        // ���� ������� �������, �������� ����������
        if (m_trlist[local]->is_active())
            m_trlist[local]->fire(lower);
        else
        {
            // ����� ������ ������� �����
            for (int j = 0; j < pl_num(); ++j)
                m_marking[j] -= m_mtxin[local][j];
            // ���������� �������
            m_trlist[local]->on_activate();
            m_trlist[local]->activate();
        };
        // ���� ������� �������� ���� ��������
        if (!m_trlist[local]->is_active())
        {
            m_trlist[local]->on_passivate();
            // ������� �������� �����
            for (int j = 0; j < pl_num(); ++j)
                m_marking[j] += m_mtxout[local][j];
        };

        // ������� ���������
        refresh();
    }
};

// ���� ����� �������� ������
class petrinet_type: public transition_compound_type
{
public:
    // ����������� ��� �����, � ������� "�����" ���� �����
    class environment_abstract_type
    {
    public:
        // �������� ������������ ������ �� ���������
        // ���������� ������ ����������� ��������� � ���������� �������
        virtual
        int wait(
            const enabledlist_type &enabled,
            const markedlist_type &marked) = 0;
    };

public:
    petrinet_type(const content_type &content):
        transition_compound_type(content)
    {}
    // ������ ��������� ���� ���� �����
    void live(environment_abstract_type &env)
    {
        activate();
        while (is_active())
            fire(env.wait(get_enabled(), get_marked()));
    }
};

} // namespace petri
} // namespace z

#endif /* _ZPETRI_HXX_ */
