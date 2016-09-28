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


#ifndef _ZPETRI_ENV_HXX_
#define _ZPETRI_ENV_HXX_

#include <vector>
#include <deque>
#include <map>
#include <utility>
#include <cstdlib>
#include <windows.h>
#include "zpetri.hxx"

namespace z {
namespace petri {

// ����� ��������� � ������������ ������� ��������
class randomenv_type: public petrinet_type::environment_abstract_type
{
public:
    // ��������� ���������������� ����� � ��������� [0, bound)
    static
    int random(int bound)
    {
        assert(bound > 0);
        return int(rand() / (RAND_MAX + 1.0) * bound);
    }
    // ����������� ������������ �������
    int wait(
        const petrinet_type::enabledlist_type &enabled,
        const petrinet_type::markedlist_type &marked)
    {
        return random(enabled.size());
    }
};

// ����� ��������� � ���������� �������������� ����������
class threadenv_type: public randomenv_type
{
public:
    // ����������� ��� ������, ����������� �� ����� ����������� ��������
    class longjob_abstract_type
    {
    public:
        // ������� ���������� ������
        virtual
        void run(void) = 0;
    };

private:
    // ������� ���������� ���������� ��������
    class transition_stop_type: public transition_simple_type
    {
    private:
        // ����� ���������������� ����������� ��������
        int m_id;
    public:
        transition_stop_type(int id): m_id(id) {}
        int id(void) const { return m_id; }
    };

    // ��������� ���������� ������
    struct threadparam_type
    {
        // ��������� �� ��������������� ������
        longjob_abstract_type *pjob;
    };

    // ��������� ������, ����������� � ������ ���������� ���������
    struct jobdata_type
    {
        // ���������� ���������������� ����������� ��������
        place_type started, stopped;
        transition_stop_type stop;
        // ����������� ���������� ������ �����
        HANDLE thread;
        // ��������� ������
        threadparam_type param;
        // �����������
        jobdata_type(int id, const threadparam_type &p): stop(id), param(p) {}
    };

public:
    // ���������� �������, ��������������� ���������� ������
    class transition_long_type: public transition_compound_type
    {
    private:
        // ���������, � ������� ������ �������
        threadenv_type *m_penv;
        // ����� �������� � ������ ���������� ��������� ���������
        int m_id;

        // ������ ������������� � ����������� ���������� ������ ����������
        void on_activate(void)
        {
            m_penv->initialize_longjob(m_id);
        }
        void on_passivate(void)
        {
            m_penv->finalize_longjob(m_id);
        }

    public:
        // ����������� ����������� ��������
        // ��������� ������ �� ����������� ������ � ��������� ���������
        // ������������� ������� ������ ������� (��� �����������)
        transition_long_type(longjob_abstract_type &longjob, threadenv_type &env):
            transition_compound_type(content_type()), m_penv(&env)
        {
            // ���������� ������ ����������� ��������
            std::pair<int, jobdata_type *> p = env.allocate_longjob(longjob);
            m_id = p.first;
            // ������������ ����������� ���������� ��������
            jobdata_type &jobdata = *p.second;
            transition_compound_type::content_type content;
            content.add_place(jobdata.started);
            content.add_place(jobdata.stopped);
            content.add_transition(jobdata.stop);
            content.add_arc(jobdata.started, jobdata.stop);
            content.add_arc(jobdata.stop, jobdata.stopped);
            content.add_token(jobdata.started);
            // ������ �������� ����������� ���������� ��������
            transition_compound_type::operator =(
                transition_compound_type(content));
        }
    };

private:
    // ��������� ������ ���������� ���������
    // � ������� ������� ������, ��������� ����� ��� ����������
    // ������ �������� ���������� ����������� ��������� �� ������
    std::deque<jobdata_type> m_alljobdata;

    // ������� ������ - ��������� ����� ������� ���������� ������
    static
    DWORD WINAPI thr_proc(LPVOID param)
    {
        threadparam_type &p = *static_cast<threadparam_type *>(param);
        // ���������� ������
        p.pjob->run();
        return 0;
    }

    // ������ ���������� ������
    void initialize_longjob(int id)
    {
        // �������� ������
        DWORD dwId;
        m_alljobdata[id].thread = ::CreateThread(
            NULL, 0,
            thr_proc, &m_alljobdata[id].param,
            0, &dwId);
        assert(m_alljobdata[id].thread != NULL);
    }
    // �������������� �������� ����� ���������� ������
    void finalize_longjob(int id)
    {
        // ������������ ��������
        // � ����� ������� ����� ��� �������� ������
        ::CloseHandle(m_alljobdata[id].thread);
    }

    // ���������� ��������� ������ ����������� ��������
    // ���������� ����������� ����� � ��������� �� ���������
    std::pair<int, jobdata_type *> allocate_longjob(
        longjob_abstract_type &longjob)
    {
        // ����� ������������ ����������� ��������
        int id = m_alljobdata.size();

        // �������� ��������� ���������� ������
        threadparam_type param = {};
        param.pjob = &longjob;
        // ��������� ������ ����������� ��������
        m_alljobdata.push_back(jobdata_type(id, param));
        // ������ ����� �������� � ��������� �� ��� ������
        return std::make_pair(id, &m_alljobdata.back());
    }

public:
    // �������� ������������ ��������
    int wait(
        const petrinet_type::enabledlist_type &enabled,
        const petrinet_type::markedlist_type &marked)
    {
        // ������ �������� ���������� ���������
        // busy - �������� ���������� �����, ������� ��� �� ���������
        // finished - �������� ���������� �����, ������� ��� ���������
        // free - ��������, �� ���������� ���������� ���������� �����
        std::vector<int> busy, finished, free;
        // ������ �������, ��������� � �������������� ��������
        std::vector<HANDLE> hdls;
        // �������� �� ����� ������ ���������
        petrinet_type::enabledlist_type::const_iterator it;
        for (it = enabled.begin(); it != enabled.end(); ++it)
        {
            transition_stop_type *stop = dynamic_cast<transition_stop_type *>(*it);
            // ���� ��� �� ������� ���������� ������, ������ ������ � ���������
            if (!stop)
                free.push_back(std::distance(enabled.begin(), it));
            else
            {
                // ����� �������� ����� ���������� ������
                HANDLE hdl = m_alljobdata[stop->id()].thread;
                // � ���������, ���������� �� ��
                if (::WaitForSingleObject(hdl, 0) == WAIT_OBJECT_0)
                    finished.push_back(std::distance(enabled.begin(), it));
                else
                {
                    // ���� �� ����������, ��������� ���������� ��� ��������
                    busy.push_back(std::distance(enabled.begin(), it));
                    hdls.push_back(hdl);
                };
            };
        };
        // ��������� ��� �������� �� ������ �����������
        int rc;
        // ���� ���� ������������� ������, ����������
        // ������ ������ �� ��������������� ���������
        if (!finished.empty())
            rc = finished[random(finished.size())];
        // ���� ���� ��������� ��������, ������ ������ ������ �� ���
        else if (!free.empty())
            rc = free[random(free.size())];
        else
        {
            // � ��������� ������� �������� ���������� ������ �� �������
            DWORD dw = ::WaitForMultipleObjects(
                hdls.size(), &hdls.front(), FALSE, INFINITE);
            assert(dw >= WAIT_OBJECT_0 && dw < WAIT_OBJECT_0 + hdls.size());
            // � ������ ������ ���������������� ��������
            rc = busy[dw - WAIT_OBJECT_0];
        };
        return rc;
    }
};

} // namespace petri
} // namespace z

#endif /* _ZPETRI_ENV_HXX_ */
