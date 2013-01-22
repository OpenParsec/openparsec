#ifndef NET_PredICTIONMANAGER_H_
#define NET_PredICTIONMANAGER_H_

// forward decls --------------------------------------------------------------
//
class NET_Stream;

// constants ------------------------------------------------------------------
//
#define MAX_NUM_PREDICTIONS	1000

// virtual base class for all prediction events -------------------------------
//
class NET_PredEvent
{
public:
					NET_PredEvent() : m_nMessageId( 0 ), m_nDoCount(0) {}
	virtual			~NET_PredEvent() {};
	virtual void	Do() { m_nDoCount++; }
	void			SetMessageId( dword nMessageId ) { m_nMessageId = nMessageId; }
	dword			GetMessageId() { return m_nMessageId; }
	int				GetDoCount()   { return m_nDoCount; }
protected:
	dword m_nMessageId;
	int	  m_nDoCount;
};


// typedefs -------------------------------------------------------------------
//
typedef UTL_listentry_s<NET_PredEvent*> LE_PredEvt;


// special implementation for an extra collect prediction event ---------------
//
class G_PredEventExtraCollect : public NET_PredEvent
{
public:
	G_PredEventExtraCollect( ExtraObject* pExtra );
	void Do();

protected:
	ExtraObject*	m_pExtra;
};


// the prediction manager -----------------------------------------------------
//
class NET_PredictionManager
{
public:
	//NOTE: stores information about events that were predicted in the game-engine
	//      whenever the game-engine modifies the local world, we save a prediction that includes a checkpoint ( = next outgoing message id )
	//		whenever we receive a packet, we clear all predictions inserted before the ACK checkpoint
	//           and we re-apply all pending predictions not yet ACK ( checkpoint # is synonym for message id )

	// INITIALIZERS
	// standard ctor
	NET_PredictionManager( NET_Stream* pStream );

	// standard dtor
	~NET_PredictionManager();


	// MODIFIERS
	// reset all data
	void Reset();

	// clear all predictions up to the message# and reapply all predictions after the message#
	void DoCheckpoint( dword MessageId );

	// predictions
	void PredictExtraCollect( ExtraObject* pExtra );

	// ACCESSORS
	// return the # of predictions pending
	int GetNumPredictions() { return m_predictions->GetNumEntries(); }

protected:
	UTL_List<NET_PredEvent*>*		m_predictions;
	NET_Stream*					m_pStream;

protected:
	// add a predictionevent
	// ensures that MAX_NUM_PREDICTIONS is not exceeded
	void _AddPrediction( NET_PredEvent* pEvent );
};

// external global ------------------------------------------------------------
//
extern NET_PredictionManager ThePredictionManager;

#endif // NET_PredICTIONMANAGER_H_
