// Pure apple stock rules. No tree prefab, player, UI or World Editor needed.
// Keeping harvest rules separate makes them easy to test in Reforger Autotest.
class LRP_AppleHarvestStock
{
	protected int m_iCapacity;
	protected int m_iRemaining;
	protected int m_iRegrowSeconds;
	protected int m_iRegrowAt;

	void Reset(int capacity, int regrowSeconds = 300)
	{
		m_iCapacity = Math.Max(capacity, 0);
		m_iRemaining = m_iCapacity;
		m_iRegrowSeconds = Math.Max(regrowSeconds, 0);
		m_iRegrowAt = 0;
	}

	int GetRemaining()
	{
		return m_iRemaining;
	}

	int GetCapacity()
	{
		return m_iCapacity;
	}

	int GetRegrowAt()
	{
		return m_iRegrowAt;
	}

	// Takes a time supplied by the caller, allowing deterministic tests.
	void Update(int nowSeconds)
	{
		if (m_iCapacity <= 0 || m_iRegrowSeconds <= 0 || m_iRegrowAt <= 0)
			return;

		if (nowSeconds < m_iRegrowAt)
			return;

		m_iRemaining = m_iCapacity;
		m_iRegrowAt = 0;
	}

	bool TryPick(int nowSeconds)
	{
		Update(nowSeconds);
		if (m_iRemaining <= 0)
			return false;

		m_iRemaining--;
		// Start the countdown only after the final apple is picked.
		if (m_iRemaining == 0 && m_iRegrowSeconds > 0)
			m_iRegrowAt = nowSeconds + m_iRegrowSeconds;

		return true;
	}
}
