package api

import (
	"errors"

	dbm "github.com/cosmos/cosmos-db"
	"github.com/initia-labs/initiavm/types"
)

/*** Mock KVStore ****/

type Lookup struct {
	db *dbm.MemDB
}

func NewLookup() *Lookup {
	return &Lookup{
		db: dbm.NewMemDB(),
	}
}

// Get wraps the underlying DB's Get method panicing on error.
func (l Lookup) Get(key []byte) []byte {
	v, err := l.db.Get(key)
	if err != nil {
		panic(err)
	}

	return v
}

// Set wraps the underlying DB's Set method panicing on error.
func (l Lookup) Set(key, value []byte) {
	if err := l.db.Set(key, value); err != nil {
		panic(err)
	}
}

// Delete wraps the underlying DB's Delete method panicing on error.
func (l Lookup) Delete(key []byte) {
	if err := l.db.Delete(key); err != nil {
		panic(err)
	}
}

// Iterator wraps the underlying DB's Iterator method panicing on error.
func (l Lookup) Iterator(start, end []byte) dbm.Iterator {
	iter, err := l.db.Iterator(start, end)
	if err != nil {
		panic(err)
	}

	return iter
}

// ReverseIterator wraps the underlying DB's ReverseIterator method panicing on error.
func (l Lookup) ReverseIterator(start, end []byte) dbm.Iterator {
	iter, err := l.db.ReverseIterator(start, end)
	if err != nil {
		panic(err)
	}

	return iter
}

var _ KVStore = (*Lookup)(nil)

/***** Mock GoAPI ****/

const CanonicalLength = 32

const (
	CostTransfer uint64 = 100
)

var _ GoAPI = MockAPI{}

type MockAPI struct {
	AccountAPI *MockAccountAPI
	StakingAPI *MockStakingAPI
	OracleAPI  *MockOracleAPI
	BlockTime  uint64
}

func NewMockAPI(
	blockTime uint64,
	accountAPI *MockAccountAPI,
	stakingAPI *MockStakingAPI,
	oracleAPI *MockOracleAPI,
) *MockAPI {

	return &MockAPI{
		AccountAPI: accountAPI,
		StakingAPI: stakingAPI,
		OracleAPI:  oracleAPI,
		BlockTime:  blockTime,
	}
}

func NewEmptyMockAPI(blockTime uint64) *MockAPI {
	accountAPI := NewMockAccountAPI()
	stakingAPI := NewMockStakingAPI()
	oracleAPI := NewMockOracleAPI()
	return &MockAPI{
		AccountAPI: &accountAPI,
		StakingAPI: &stakingAPI,
		OracleAPI:  &oracleAPI,
		BlockTime:  blockTime,
	}
}

func (m MockAPI) GetAccountInfo(addr types.AccountAddress) (bool, uint64, uint64, uint8) {
	return m.AccountAPI.GetAccountInfo(addr)
}

func (m MockAPI) AmountToShare(validator []byte, metadata types.AccountAddress, amount uint64) (uint64, error) {
	return m.StakingAPI.AmountToShare(validator, metadata, amount)
}

func (m MockAPI) ShareToAmount(validator []byte, metadata types.AccountAddress, share uint64) (uint64, error) {
	return m.StakingAPI.ShareToAmount(validator, metadata, share)
}

func (m MockAPI) UnbondTimestamp() uint64 {
	return m.BlockTime + 60*60*24*7
}

func (m MockAPI) GetPrice(pairId string) ([]byte, uint64, uint64, error) {
	return m.OracleAPI.GetPrice(pairId)
}

type MockAccountAPI struct {
	accounts map[string][]uint64
}

// NewMockAccountAPI return MockAccountAPI instance
func NewMockAccountAPI() MockAccountAPI {
	return MockAccountAPI{
		accounts: make(map[string][]uint64),
	}
}

func (m *MockAccountAPI) SetAccountInfo(addr types.AccountAddress, accountNumber, sequence uint64, accountType uint8) {
	m.accounts[addr.String()] = []uint64{accountNumber, sequence, uint64(accountType)}
}

func (m MockAccountAPI) GetAccountInfo(addr types.AccountAddress) (bool, uint64, uint64, uint8) {
	info, found := m.accounts[addr.String()]
	if found {
		return found, info[0], info[1], uint8(info[2])
	}

	return false, 0, 0, 0
}

type ShareAmountRatio struct {
	share  uint64
	amount uint64
}

type MockStakingAPI struct {
	validators map[string]map[types.AccountAddress]ShareAmountRatio
}

// NewMockStakingAPI return MockStakingAPI instance
func NewMockStakingAPI() MockStakingAPI {
	return MockStakingAPI{
		validators: make(map[string]map[types.AccountAddress]ShareAmountRatio),
	}
}

func (m *MockStakingAPI) SetShareRatio(validator []byte, metadata types.AccountAddress, share uint64, amount uint64) {
	if ratios, ok := m.validators[string(validator)]; ok {
		ratios[metadata] = ShareAmountRatio{share, amount}
	} else {
		m.validators[string(validator)] = make(map[types.AccountAddress]ShareAmountRatio)
		m.validators[string(validator)][metadata] = ShareAmountRatio{share, amount}
	}
}

func (m MockStakingAPI) AmountToShare(validator []byte, metadata types.AccountAddress, amount uint64) (uint64, error) {
	ratios, ok := m.validators[string(validator)]
	if !ok {
		return 0, errors.New("validator not found")
	}

	ratio, ok := ratios[metadata]
	if !ok {
		return 0, errors.New("metadata not found")
	}

	return amount * ratio.share / ratio.amount, nil
}

func (m MockStakingAPI) ShareToAmount(validator []byte, metadata types.AccountAddress, share uint64) (uint64, error) {
	ratios, ok := m.validators[string(validator)]
	if !ok {
		return 0, errors.New("validator not found")
	}

	ratio, ok := ratios[metadata]
	if !ok {
		return 0, errors.New("metadata not found")
	}

	return share * ratio.amount / ratio.share, nil
}

type MockOracleAPI struct {
	prices map[string][]uint64
}

// NewMockOracleAPI return MockOracleAPI instance
func NewMockOracleAPI() MockOracleAPI {
	return MockOracleAPI{
		prices: make(map[string][]uint64),
	}
}

func (m *MockOracleAPI) SetPrice(pairId string, price, updatedAt, decimals uint64) {
	m.prices[pairId] = []uint64{price, updatedAt, decimals}
}

func (m MockOracleAPI) GetPrice(pairId string) ([]byte, uint64, uint64, error) {
	info, found := m.prices[pairId]
	if !found {
		return nil, 0, 0, errors.New("pair not found")
	}

	priceBz, err := types.SerializeUint256(0, 0, 0, info[0])
	if err != nil {
		return nil, 0, 0, err
	}

	return priceBz, info[1], info[2], nil
}
